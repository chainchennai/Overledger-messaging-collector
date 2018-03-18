/*
 * ConfigMgrPimpl.cpp
 *
 *  Created on: Sep 2, 2012
 *      Author: admin
 */

#include "ConfigMgrPimpl.h"
#include "ConfigMgr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>

#define BACKLOG 10 // how many pending connections queue will hold
void sigchld_handler(int s) {
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

CConfigMgrPimpl::CConfigMgrPimpl(CConfigMgr* pMgr) :
	m_thread_id(0), m_pMgr(pMgr) {
	pthread_create(&m_thread_id, 0, &ListenerThread, (void*) this);
}

CConfigMgrPimpl::~CConfigMgrPimpl() {
	pthread_cancel(m_thread_id);
	void *res;
	pthread_join(m_thread_id, &res);
}

void* CConfigMgrPimpl::ListenerThread(void* lp) {
	CConfigMgrPimpl* _pImpl = (CConfigMgrPimpl*) lp;
	if (_pImpl == 0) return 0;
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, _pImpl->m_pMgr->Port().c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}
	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("setsockopt");
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 0;
	}
	freeaddrinfo(servinfo); // all done with this structure
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
	}
	while (1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr(
				(struct sockaddr *) &their_addr), s, sizeof s);
		//if (!fork()) { // this is the child process
		//	close(sockfd); // child doesn't need the listener
		_pImpl->new_connection(new_fd);
		//	close(new_fd);
		//	exit(0);
		//}
		close(new_fd); // parent doesn't need this
	}
	return 0;
}

void CConfigMgrPimpl::recv_config(int fd) {
	int numbytes;
	char buffer[4096];
	if ((numbytes = recv(fd, buffer, 4095, 0)) != -1) {
		buffer[numbytes] = '\0';
		FILE * conf = fopen(m_pMgr->ConfigFileName().c_str(), "w");
		if (conf) {
			fprintf(conf, buffer);
			fclose(conf);
			m_pMgr->Refresh();
		}
	}
}

void CConfigMgrPimpl::new_connection(int fd) {
	int numbytes;
	char buffer[4096];
	if ((numbytes = recv(fd, buffer, 1, 0)) != -1) {
		buffer[numbytes] = '\0';

		switch (buffer[0]) {
		case 49:
			if ((numbytes = recv(fd, buffer, 1, 0)) != -1) {
				buffer[numbytes] = '\0';
				switch (buffer[0]) {
				case 49: // save new config
					this->recv_config(fd);
					break;
				case 50: // sand existed config
					this->send_config(fd);
					break;
				default:
					;
				}
			}
			break;
		default:
			;// unknown protocol
		}

	}

}

int CConfigMgrPimpl::sendall(int s, char *buf, int *len) {
	int total = 0;
	// how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;
	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}
	*len = total; // return number actually sent here
	return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void CConfigMgrPimpl::send_config(int fd) {
	using namespace std;
	char * buffer;     //buffer to store file contents
	int size;     //file size
	ifstream file (m_pMgr->ConfigFileName().c_str(), ios::in|ios::binary|ios::ate);     //open file in binary mode, get pointer at the end of the file (ios::ate)
	size = file.tellg();     //retrieve get pointer position
	file.seekg (0, ios::beg);     //position get pointer at the begining of the file
	buffer = new char [size];     //initialize the buffer
	file.read (buffer, size);     //read file to buffer
	file.close();     //close file
	this->sendall(fd, buffer, &size);
	delete [] buffer;
}
