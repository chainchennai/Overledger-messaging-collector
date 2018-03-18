/*
 * CProcess.cpp
 *
 *  Created on: Aug 30, 2012
 *      Author: admin
 */

#include "Process.h"
#include <cstring>
#include <stdio.h>

long long CProcess::IOReadCurrent = 0;
long long CProcess::IOReadOld = 0;
long long CProcess::IOReadDelta = 0;
long long CProcess::IOWriteCurrent = 0;
long long CProcess::IOWriteOld = 0;
long long CProcess::IOWriteDelta = 0;

CProcess::CProcess(pid_t pid) {
	m_pid = pid;
	memset(&m_lasttime, 0, sizeof(timeval));
	memset(&m_timediff, 0, sizeof(timeval));
	m_otime = 0;

	struct SStat s;
	memset(&s, 0, sizeof(SStat));
	this->ReadStat(&s);

	m_sName = s.comm;
}

CProcess::CProcess(const CProcess & proc) {
	m_pid = proc.m_pid;
	m_lasttime = proc.m_lasttime;
	m_timediff = proc.m_timediff;
	m_otime = proc.m_otime;
	m_sName = proc.m_sName;

	m_vecConnections.clear();
	m_vecConnections = proc.m_vecConnections;
}

CProcess::~CProcess() {
	// TODO Auto-generated destructor stub
}

int CProcess::GetCPU() {
	struct timeval thistime;
	gettimeofday(&thistime, 0);
	timersub(&thistime, &m_lasttime, &m_timediff);
	m_lasttime = thistime;

	unsigned long now;
	unsigned long elapsed;

	now = (unsigned long) m_lasttime.tv_sec;
	if (m_lasttime.tv_usec >= 500000) {
		now++;
	}

	elapsed = m_timediff.tv_sec * 100 + (m_timediff.tv_usec * 100) / 1000000;
	if (elapsed <= 0) {
		elapsed = 1;
	}

	struct SStat s;
	memset(&s, 0, sizeof(SStat));
	this->ReadStat(&s);

	double pcpu = (s.utime + s.stime - m_otime) / (double) elapsed * 100.0;
	if (pcpu > 99) {
		pcpu = 99;
	} else if (pcpu < 0) {
		pcpu = 0;
	}

	m_otime = s.utime + s.stime;

	return pcpu;
}

int CProcess::GetMem() {
	struct SStatus s;
	memset(&s, 0, sizeof(SStatus));
	this->ReadStatus(&s);

	int ram = 0;

	FILE *meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL) {
		return -1;
	}

	char line[256];
	while (fgets(line, sizeof(line), meminfo)) {
		if (sscanf(line, "MemTotal: %d kB", &ram) == 1) {
			break;
		}
	}

	fclose(meminfo);

	if (ram) {
		return double(s.VmRSS * 100) / ram;
	} else {
		return -1;
	}
}

void CProcess::GetReadWriteBytes(unsigned long &r, unsigned long &w) {
	struct SIO io;
	memset(&io, 0, sizeof(SIO));
	this->ReadIO(&io);

	r = io.read_bytes;
	w = io.write_bytes;
}

void CProcess::ReadStat(SStat * s) {

	const char
			*format =
					"%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu %llu";
	char buf[256];
	FILE *proc;
	sprintf(buf, "/proc/%d/stat", m_pid);
	proc = fopen(buf, "r");
	if (proc) {
		if (42 == fscanf(proc, format, &s->pid, s->comm, &s->state, &s->ppid,
				&s->pgrp, &s->session, &s->tty_nr, &s->tpgid, &s->flags,
				&s->minflt, &s->cminflt, &s->majflt, &s->cmajflt, &s->utime,
				&s->stime, &s->cutime, &s->cstime, &s->priority, &s->nice,
				&s->num_threads, &s->itrealvalue, &s->starttime, &s->vsize,
				&s->rss, &s->rlim, &s->startcode, &s->endcode, &s->startstack,
				&s->kstkesp, &s->kstkeip, &s->signal, &s->blocked,
				&s->sigignore, &s->sigcatch, &s->wchan, &s->nswap, &s->cnswap,
				&s->exit_signal, &s->processor, &s->rt_priority, &s->policy,
				&s->delayacct_blkio_ticks)) {
			fclose(proc);
		} else {
			fclose(proc);
		}
	}
}

void CProcess::ReadStatus(SStatus * s) {
	int i;
	char name[256];
	char buf[256];
	FILE *proc;
	sprintf(name, "/proc/%d/status", m_pid);
	proc = fopen(name, "r");
	if (proc) {
		// Name:	tcsh
		fgets(buf, 256, proc);
		sscanf(buf, "Name:\t%s", s->Name);
		// State:	S (sleeping)
		fgets(buf, 256, proc);
		sscanf(buf, "State:\t%c", &s->State);
		// SleepAVG:	98%
		fgets(buf, 256, proc);
		sscanf(buf, "SleepAVG:\t%lu", &s->SleepAVG);
		// Tgid:	20616
		fgets(buf, 256, proc);
		sscanf(buf, "Tgid:\t%lu", &s->Tgid);
		// Pid:	20616
		fgets(buf, 256, proc);
		sscanf(buf, "Pid:\t%lu", &s->Pid);
		// PPid:	20612
		fgets(buf, 256, proc);
		sscanf(buf, "PPid:\t%lu", &s->PPid);
		// TracerPid:	0
		fgets(buf, 256, proc);
		sscanf(buf, "TracerPid:\t%lu", &s->TracerPid);
		// Uid:	418	418	418	418
		fgets(buf, 256, proc);
		sscanf(buf, "Uid:\t%lu\t%lu\t%lu\t%lu", s->Uid, s->Uid + 1, s->Uid + 2,
				s->Uid + 3);
		// Gid:	30	30	30	30
		fgets(buf, 256, proc);
		sscanf(buf, "Gid:\t%lu\t%lu\t%lu\t%lu", s->Gid, s->Gid + 1, s->Gid + 2,
				s->Gid + 3);
		// FDSize:	64
		fgets(buf, 256, proc);
		sscanf(buf, "FDSize:\t%lu", &s->FDSize);
		// Groups:	30 118 121 136 148 260 262 724 728 60045 60053 60072 600159 600217 600241 600245
		fgets(buf, 256, proc);
		for (i = 0; i < 16; i++)
			s->Groups[i] = 0;
		i = sscanf(buf,
				"Groups:\t%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld",
				s->Groups, s->Groups + 1, s->Groups + 2, s->Groups + 3,
				s->Groups + 4, s->Groups + 5, s->Groups + 6, s->Groups + 7,
				s->Groups + 8, s->Groups + 9, s->Groups + 10, s->Groups + 11,
				s->Groups + 12, s->Groups + 13, s->Groups + 14, s->Groups + 15);
		// VmPeak:	   64732 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmPeak:\t%lu", &s->VmPeak);
		// VmSize:	   64700 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmSize:\t%lu", &s->VmSize);
		// VmLck:	       0 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmLck:\t%lu", &s->VmLck);
		// VmHWM:	    1756 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmHWM:\t%lu", &s->VmHWM);
		// VmRSS:	    1756 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmRSS:\t%lu", &s->VmRSS);
		// VmData:	    1112 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmData:\t%lu", &s->VmData);
		// VmStk:	     348 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmStk:\t%lu", &s->VmStk);
		// VmExe:	     320 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmExe:\t%lu", &s->VmExe);
		// VmLib:	    1496 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmLib:\t%lu", &s->VmLib);
		// VmPTE:	      68 kB
		fgets(buf, 256, proc);
		sscanf(buf, "VmPTE:\t%lu", &s->VmPTE);
		// StaBrk:	0871a000 kB
		fgets(buf, 256, proc);
		sscanf(buf, "StaBrk:\t%lx", &s->StaBrk);
		// Brk:	0879b000 kB
		fgets(buf, 256, proc);
		sscanf(buf, "Brk:\t%lx", &s->Brk);
		// StaStk:	7fff6d0ccc70 kB
		fgets(buf, 256, proc);
		sscanf(buf, "StaStk:\t%lx", &s->StaStk);
		// Threads:	1
		fgets(buf, 256, proc);
		sscanf(buf, "Threads:\t%lu", &s->Threads);
		// SigQ:	1/16368
		fgets(buf, 256, proc);
		sscanf(buf, "SigQ:\t%lu/%lu", s->SigQ, s->SigQ + 1);
		// SigPnd:	0000000000000000
		fgets(buf, 256, proc);
		sscanf(buf, "SigPnd:\t%lx", &s->SigPnd);
		// ShdPnd:	0000000000000000
		fgets(buf, 256, proc);
		sscanf(buf, "ShdPnd:\t%lx", &s->ShdPnd);
		// SigBlk:	0000000000000002
		fgets(buf, 256, proc);
		sscanf(buf, "SigBlk:\t%lx", &s->SigBlk);
		// SigIgn:	0000000000384004
		fgets(buf, 256, proc);
		sscanf(buf, "SigIgn:\t%lx", &s->SigIgn);
		// SigCgt:	0000000009812003
		fgets(buf, 256, proc);
		sscanf(buf, "SigCgt:\t%lx", &s->SigCgt);
		// CapInh:	0000000000000000
		fgets(buf, 256, proc);
		sscanf(buf, "CapInh:\t%lx", &s->CapInh);
		// CapPrm:	0000000000000000
		fgets(buf, 256, proc);
		sscanf(buf, "CapPrm:\t%lx", &s->CapPrm);
		// CapEff:	0000000000000000
		fgets(buf, 256, proc);
		sscanf(buf, "CapEff:\t%lx", &s->CapEff);
		// Cpus_allowed:	00000000,00000000,00000000,00000000,00000000,00000000,00000000,000000ff
		fgets(buf, 256, proc);
		for (i = 0; i < 8; i++)
			s->Cpus_allowed[i] = 0;
		sscanf(buf, "Cpus_allowed:\t%lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx",
				s->Cpus_allowed, s->Cpus_allowed + 1, s->Cpus_allowed + 2,
				s->Cpus_allowed + 3, s->Cpus_allowed + 4, s->Cpus_allowed + 5,
				s->Cpus_allowed + 6, s->Cpus_allowed + 7);
		// Mems_allowed:	00000000,00000001
		fgets(buf, 256, proc);
		sscanf(buf, "Mems_allowed:\t%lx,%lx", &(s->Mems_allowed[0]),
				&(s->Mems_allowed[1]));
		fclose(proc);
	}
}

void CProcess::ReadIO(SIO * io) {
	char name[256];
	char buf[256];
	FILE *proc;
	sprintf(name, "/proc/%d/io", m_pid);
	proc = fopen(name, "r");
	if (proc) {
		// rchar
		fgets(buf, 256, proc);
		sscanf(buf, "rchar:\t%lu", &io->rchar);
		// wchar
		fgets(buf, 256, proc);
		sscanf(buf, "wchar:\t%lu", &io->wchar);
		// syscr
		fgets(buf, 256, proc);
		sscanf(buf, "syscr:\t%lu", &io->syscr);
		// syscw
		fgets(buf, 256, proc);
		sscanf(buf, "syscw:\t%lu", &io->syscw);
		// read_bytes
		fgets(buf, 256, proc);
		sscanf(buf, "read_bytes:\t%lu", &io->read_bytes);
		// write_bytes
		fgets(buf, 256, proc);
		sscanf(buf, "write_bytes:\t%lu", &io->write_bytes);
		// cancelled_write_bytes
		fgets(buf, 256, proc);
		sscanf(buf, "cancelled_write_bytes:\t%lu", &io->cancelled_write_bytes);

		fclose(proc);
	}
}
