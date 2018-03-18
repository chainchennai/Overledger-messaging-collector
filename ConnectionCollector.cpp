/*
 * ConnectionCollector.cpp
 *
 *  Created on: Aug 30, 2017
 *      Author: admin
 */

#include "ConnectionCollector.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <dirent.h>
CConnectionCollector::CConnectionCollector() {
	// TODO Auto-generated constructor stub

}

CConnectionCollector::~CConnectionCollector() {
	// TODO Auto-generated destructor stub
}

void CConnectionCollector::ReadConnections(const char * file,
		SConnection::eConnectionType type,
		std::map<unsigned long, SConnection> &mapConnInode) {

	FILE * procinfo = fopen(file, "r");

	char buffer[8192];

	if (procinfo == NULL)
		return;

	fgets(buffer, sizeof(buffer), procinfo);

	do {
		if (fgets(buffer, sizeof(buffer), procinfo)) {
			short int sa_family;
			struct in6_addr result_addr_local;
			struct in6_addr result_addr_remote;

			char rem_addr[128], local_addr[128];
			int local_port, rem_port;
			struct in6_addr in6_local;
			struct in6_addr in6_remote;

			unsigned long inode;

			int
					matches =
							sscanf(
									buffer,
									"%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X %*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
									local_addr, &local_port, rem_addr,
									&rem_port, &inode);

			if (matches != 5) {
				fprintf(stderr, "Unexpected buffer: '%s'\n", buffer);
				return;
			}

			if (inode == 0) {
				return;
			}

			if (strlen(local_addr) > 8) {
				sscanf(local_addr, "%08X%08X%08X%08X", &in6_local.s6_addr32[0],
						&in6_local.s6_addr32[1], &in6_local.s6_addr32[2],
						&in6_local.s6_addr32[3]);
				sscanf(rem_addr, "%08X%08X%08X%08X", &in6_remote.s6_addr32[0],
						&in6_remote.s6_addr32[1], &in6_remote.s6_addr32[2],
						&in6_remote.s6_addr32[3]);

				if ((in6_local.s6_addr32[0] == 0x0) && (in6_local.s6_addr32[1]
						== 0x0) && (in6_local.s6_addr32[2] == 0xFFFF0000)) {
					result_addr_local
							= *((struct in6_addr*) &(in6_local.s6_addr32[3]));
					result_addr_remote
							= *((struct in6_addr*) &(in6_remote.s6_addr32[3]));
					sa_family = AF_INET;
				}
			} else {
				sscanf(local_addr, "%X", (unsigned int *) &result_addr_local);
				sscanf(rem_addr, "%X", (unsigned int *) &result_addr_remote);
				sa_family = AF_INET;
			}

			char local_string[50];
			char remote_string[50];
			inet_ntop(sa_family, &result_addr_local, local_string, 49);
			inet_ntop(sa_family, &result_addr_remote, remote_string, 49);

			mapConnInode[inode] = SConnection(type, local_string, local_port,
					remote_string, rem_port);
		}

	} while (!feof(procinfo));

	fclose(procinfo);
}

void CConnectionCollector::Refresh() {

	std::map<unsigned long, SConnection> mapConnInode;

	this->ReadConnections("/proc/net/tcp", SConnection::eType_TCP, mapConnInode);
	this->ReadConnections("/proc/net/udp", SConnection::eType_UDP, mapConnInode);

	m_mapConnInode.clear();
	m_mapConnInode = mapConnInode;
}

void CConnectionCollector::GetConnectionsForProcessID(pid_t pid, std::vector<SConnection> &vecConnections) {

	char buffer[512];
	memset(buffer, 0, 512);
	snprintf(buffer, 512, "/proc/%d/fd", pid);

	std::string sdir = buffer;

	DIR * dir = opendir(sdir.c_str());

	if (!dir) {
		return;
	}

	dirent * entry;
	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_LNK)
			continue;

		std::string sfrom = sdir + "/" + entry->d_name;

		int linklen = 80;
		char linkname[linklen];
		int usedlen = readlink(sfrom.c_str(), linkname, linklen - 1);
		if (usedlen == -1) {
			continue;
		}

		linkname[usedlen] = '\0';

		if (strncmp(linkname, "socket:[", 8) == 0) {
			char * ptr = linkname + 8;
			unsigned long inode = strtoul(ptr, 0, 0);

			if (m_mapConnInode.find(inode) != m_mapConnInode.end()) {
				vecConnections.push_back(m_mapConnInode[inode]);
			}
		}
	}
	closedir(dir);
}
