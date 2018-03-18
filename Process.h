/*
 * CProcess.h
 *
 *  Created on: Aug 30, 2017
 *      Author: admin
 */

#ifndef CPROCESS_H_
#define CPROCESS_H_

#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "ConnectionCollector.h"
class CProcess {

	struct SStat {
		int pid; // %d
		char comm[256]; // %s
		char state; // %c
		int ppid; // %d
		int pgrp; // %d
		int session; // %d
		int tty_nr; // %d
		int tpgid; // %d
		unsigned long flags; // %lu
		unsigned long minflt; // %lu
		unsigned long cminflt; // %lu
		unsigned long majflt; // %lu
		unsigned long cmajflt; // %lu
		unsigned long utime; // %lu
		unsigned long stime; // %lu
		long cutime; // %ld
		long cstime; // %ld
		long priority; // %ld
		long nice; // %ld
		long num_threads; // %ld
		long itrealvalue; // %ld
		unsigned long starttime; // %lu
		unsigned long vsize; // %lu
		long rss; // %ld
		unsigned long rlim; // %lu
		unsigned long startcode; // %lu
		unsigned long endcode; // %lu
		unsigned long startstack; // %lu
		unsigned long kstkesp; // %lu
		unsigned long kstkeip; // %lu
		unsigned long signal; // %lu
		unsigned long blocked; // %lu
		unsigned long sigignore; // %lu
		unsigned long sigcatch; // %lu
		unsigned long wchan; // %lu
		unsigned long nswap; // %lu
		unsigned long cnswap; // %lu
		int exit_signal; // %d
		int processor; // %d
		unsigned long rt_priority; // %lu
		unsigned long policy; // %lu
		unsigned long long delayacct_blkio_ticks; // %llu
	};

	struct SStatus {
		char Name[256]; // tcsh
		char State; // S (sleeping)
		unsigned long SleepAVG; //	98%
		unsigned long Tgid; //	20616
		unsigned long Pid; //	20616
		unsigned long PPid; //	20612
		unsigned long TracerPid; //	0
		unsigned long Uid[4]; //	418	418	418	418
		unsigned long Gid[4]; //	30	30	30	30
		unsigned long FDSize; //	64
		unsigned long Groups[16]; //	30 118 121 136 148 260 262 724 728 60045 60053 60072 600159 600217 600241 600245
		unsigned long VmPeak; //	   64732 kB
		unsigned long VmSize; //	   64700 kB
		unsigned long VmLck; //	       0 kB
		unsigned long VmHWM; //	    1756 kB
		unsigned long VmRSS; //	    1756 kB
		unsigned long VmData; //	    1112 kB
		unsigned long VmStk; //	     348 kB
		unsigned long VmExe; //	     320 kB
		unsigned long VmLib; //	    1496 kB
		unsigned long VmPTE; //	      68 kB
		unsigned long StaBrk; //	0871a000 kB
		unsigned long Brk; //	0879b000 kB
		unsigned long StaStk; //	7fff6d0ccc70 kB
		unsigned long Threads; //	1
		unsigned long SigQ[2]; //	1/16368
		unsigned long SigPnd; //	0000000000000000
		unsigned long ShdPnd; //	0000000000000000
		unsigned long SigBlk; //	0000000000000002
		unsigned long SigIgn; //	0000000000384004
		unsigned long SigCgt; //	0000000009812003
		unsigned long CapInh; //	0000000000000000
		unsigned long CapPrm; //	0000000000000000
		unsigned long CapEff; //	0000000000000000
		unsigned long Cpus_allowed[8]; //	00000000,00000000,00000000,00000000,00000000,00000000,00000000,000000ff
		unsigned long Mems_allowed[2]; //	00000000,00000001
	};

	struct SIO {
		unsigned long rchar;
		unsigned long wchar;
		unsigned long syscr;
		unsigned long syscw;
		unsigned long read_bytes;
		unsigned long write_bytes;
		unsigned long cancelled_write_bytes;
	};

private:
	pid_t m_pid;
	timeval m_lasttime;
	timeval m_timediff;
	unsigned long m_otime;
	std::string m_sName;

public:
	CProcess(pid_t pid);
	CProcess(const CProcess & proc);
	virtual ~CProcess();

	std::vector<CConnectionCollector::SConnection> m_vecConnections;

	int GetCPU();
	int GetMem();
	void GetReadWriteBytes(unsigned long &r, unsigned long &w);
	std::string GetProcessName() {
		return m_sName;
	}

	static long long IOReadCurrent;
	static long long IOReadOld;
	static long long IOReadDelta;
	static long long IOWriteCurrent;
	static long long IOWriteOld;
	static long long IOWriteDelta;
private:
	void ReadStat(SStat * s);
	void ReadStatus(SStatus * s);
	void ReadIO(SIO * io);
};

#endif /* CPROCESS_H_ */
