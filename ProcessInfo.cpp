/*
 * ProcessInfo.cpp
 *
 *  Created on: Aug 8, 2012
 *      Author: admin
 */

#include "ProcessInfo.h"
#include "Exceptions.h"
#include "ConnectionCollector.h"
#include "Process.h"

#include <sys/statfs.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define PROC_SUPER_MAGIC 0x9fa0
#define SIZE_4096 4096

CProcessInfo::CProcessInfo() {
	struct ::statfs sb;
	if (statfs("/proc", &sb) < 0 || sb.f_type != PROC_SUPER_MAGIC) {
		throw Util::CExceptionProcfsNotFound("/proc");
	}
}

CProcessInfo::~CProcessInfo() {

}

void CProcessInfo::CollectProcesses(std::vector<CProcess> &vecProcesses) {

	CConnectionCollector cc;
	cc.Refresh();

	vecProcesses.clear();

	DIR *dir = opendir("/proc");
	struct dirent *ent;

	while ((ent = readdir(dir)) != NULL) {

		pid_t pid;

		char buffer[SIZE_4096];
		int fd, len;
		std::string cmd;

		if (!isdigit(ent->d_name[0]))
			continue;

		pid = atoi(ent->d_name);

		sprintf(buffer, "/proc/%d/cmdline", pid);
		if ((fd = open(buffer, O_RDONLY)) != -1) {
			if ((len = read(fd, buffer, SIZE_4096)) > 1) {
				buffer[len] = '\0';
				cmd = buffer;
			}

			close(fd);
		}

		CProcess proc(pid);
		cc.GetConnectionsForProcessID(pid, proc.m_vecConnections);

		vecProcesses.push_back(proc);

	}
}
