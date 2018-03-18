#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "sys/sysinfo.h"
#include <signal.h>

#include "ProcessInfo.h"
#include "Process.h"
#include "Exceptions.h"
#include "ConfigMgr.h"
#include "CSnapShotDiff.h"

#define LOG(x) std::cout << x << std::endl
#define LINUX_FULL 3 // snapshot type
#define SNAPSHOT_DIFF 2

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys,
		lastTotalIdle;

double GetCPU() {
	double percent;
	FILE* file;
	unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

	file = fopen("/proc/stat", "r");
	fscanf(file, "cpu %Ld %Ld %Ld %Ld", &totalUser, &totalUserLow, &totalSys,
			&totalIdle);
	fclose(file);

	if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow
			|| totalSys < lastTotalSys || totalIdle < lastTotalIdle) {
		percent = -1.0;
	} else {
		total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow)
				+ (totalSys - lastTotalSys);
		percent = total;
		total += (totalIdle - lastTotalIdle);
		percent /= total;
		percent *= 100;
	}

	lastTotalUser = totalUser;
	lastTotalUserLow = totalUserLow;
	lastTotalSys = totalSys;
	lastTotalIdle = totalIdle;

	return percent;
}

long long GetMemory() {

	struct sysinfo memInfo;
	sysinfo(&memInfo);
	long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;

	return virtualMemUsed;

}

std::string CReateXML(CConfigMgr &conf, std::vector<CProcess> &vecProcesses) {
	std::string strGeneratedXML;
	std::stringstream outXML;

	std::string strConnectionType;
	std::string strLocalAddr;
	int nLocalPortXML;
	std::string strRemoteAddr;
	int nRemotePortXML;

	CProcess::IOReadCurrent = 0;
	CProcess::IOReadDelta = 0;
	CProcess::IOWriteCurrent = 0;
	CProcess::IOWriteDelta = 0;
	for (std::vector<CProcess>::iterator it = vecProcesses.begin(); it
			!= vecProcesses.end(); it++) {
		unsigned long r, w;
		it->GetReadWriteBytes(r, w);
		CProcess::IOReadCurrent += r;
		CProcess::IOWriteCurrent += w;

		outXML << "<Component>\n";
		outXML << "<Metrics>\n";

		// Add proc name
		outXML << "<ProcessName>";
		outXML << it->GetProcessName();
		outXML << "</ProcessName>\n";

		//AddCPU
		if(conf.CPU())
		{
			outXML << "<CPU>";
			outXML << it->GetCPU();
			outXML << "</CPU>\n";
		}

		//Add Memory
		if(conf.Mem())
		{
			outXML << "<MEM>";
			outXML << it->GetMem();
			outXML << "</MEM>\n";
		}

		//Add read bytes
		if(conf.Read())
		{
			outXML << "<RB>";
			outXML << r;
			outXML << "</RB>\n";
		}

		//Add write bytes
		if(conf.Write())
		{
			outXML << "<WB>";
			outXML << w;
			outXML << "</WB>\n";
		}

		outXML << "</Metrics>\n";

		outXML << "<Connections>\n";

		for (std::vector<CConnectionCollector::SConnection>::iterator ps =
				it->m_vecConnections.begin(); ps != it->m_vecConnections.end(); ps++) {
			outXML << "<Connection>\n";

			ps->print(strConnectionType, strLocalAddr, nLocalPortXML,
					strRemoteAddr, nRemotePortXML);

			outXML << "<ConnectionType>" << strConnectionType
					<< "</ConnectionType>\n";
			outXML << "<LocalAddress>" << strLocalAddr << "</LocalAddress>\n";
			outXML << "<LocalPort>" << nLocalPortXML << "</LocalPort>\n";
			outXML << "<RemoteAddress>" << strRemoteAddr
					<< "</RemoteAddress>\n";
			outXML << "<RemotePort>" << nRemotePortXML << "</RemotePort>\n";

			outXML << "</Connection>\n";
		}

		outXML << "</Connections>\n";

		outXML << "</Component>\n";
	}

	if (CProcess::IOReadCurrent > CProcess::IOReadOld) {
		if (CProcess::IOReadOld != 0) {
			CProcess::IOReadDelta = CProcess::IOReadCurrent
					- CProcess::IOReadOld;
		}
		CProcess::IOReadOld = CProcess::IOReadCurrent;
	}
	if (CProcess::IOWriteCurrent > CProcess::IOWriteOld) {
		if (CProcess::IOWriteOld != 0) {
			CProcess::IOWriteDelta = CProcess::IOWriteCurrent
					- CProcess::IOWriteOld;
		}
		CProcess::IOWriteOld = CProcess::IOWriteCurrent;
	}

	if(conf.SysCPU())
		outXML << "<ProcessorCPU>" << GetCPU() << "</ProcessorCPU>\n";

	if(conf.SysMem())
		outXML << "<UsedVirtualMemory>" << GetMemory() << "</UsedVirtualMemory>\n";

	if(conf.SysRead())
		outXML << "<IOReadDelta>" << CProcess::IOReadDelta << "</IOReadDelta>\n";

	if(conf.SysWrite())
		outXML << "<IOWriteDelta>" << CProcess::IOWriteDelta << "</IOWriteDelta>\n";

	strGeneratedXML = outXML.str();

	return strGeneratedXML;
}

void SendFullSnapShot(CConfigMgr &conf, std::vector<CProcess>& vecProcesses,
		CSnapShotDiff& SnapshotDiff) {
	std::string strGeneratedXML;
	std::stringstream outXML;
	char *dn =  NULL;
	char hostname[254];
	hostname[253] = '\0';

	gethostname(hostname, 1023);
	dn = strchr(hostname, '.');

	outXML << "<?xml version='1.0'?>\n";
	outXML << "<Resources>\n";
	outXML << "<LocalhostName>" << hostname << "</LocalhostName>\n";
	outXML << "<DomainName>" << ++dn << "</DomainName>\n";
	outXML << "<SnapshotType>" << LINUX_FULL << "</SnapshotType>\n";
	outXML << CReateXML(conf, vecProcesses);
	outXML << "</Resources>";
	strGeneratedXML = outXML.str();

	SnapshotDiff.SetOldProcesesList(vecProcesses);

	struct sockaddr_in Server_Address;
	int Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket == -1) {
		printf("Can Not Create A Socket!");
	}
	int Port = atoi(conf.ServerPort().c_str());
	Server_Address.sin_family = AF_INET;
	Server_Address.sin_port = htons(Port);
	Server_Address.sin_addr.s_addr = inet_addr(conf.ServerURL().c_str());
	if (Server_Address.sin_addr.s_addr == INADDR_NONE) {
		printf("Bad Address!");
	}
	connect(Socket, (struct sockaddr *) &Server_Address, sizeof(Server_Address));
	send(Socket, strGeneratedXML.c_str(), strGeneratedXML.length(), 0);
	close(Socket);
}

void SendDiffSnapshot(CConfigMgr &conf, std::vector<CProcess>& vecProcesses,
		CSnapShotDiff& SnapshotDiff) {
	std::string strGeneratedXML;
	std::stringstream outXML;
	char *dn =  NULL;
	char hostname[254];
	hostname[253] = '\0';

	gethostname(hostname, 1023);
	dn = strchr(hostname, '.');

	outXML << "<?xml version='1.0'?>\n";
	outXML << "<Resources>\n";
	outXML << "<LocalhostName>" << hostname << "</LocalhostName>\n";
	outXML << "<DomainName>" << ++dn << "</DomainName>\n";
	outXML << "<SnapshotType>" << SNAPSHOT_DIFF << "</SnapshotType>";
	outXML << SnapshotDiff.CreateDiffXML(conf, vecProcesses);

	if(conf.SysCPU())
		outXML << "<ProcessorCPU>" << GetCPU() << "</ProcessorCPU>\n";

	if(conf.SysMem())
		outXML << "<UsedVirtualMemory>" << GetMemory() << "</UsedVirtualMemory>\n";

	if(conf.SysRead())
		outXML << "<IOReadDelta>" << CProcess::IOReadDelta << "</IOReadDelta>\n";

	if(conf.SysWrite())
		outXML << "<IOWriteDelta>" << CProcess::IOWriteDelta << "</IOWriteDelta>\n";

	outXML << "</Resources>";
	strGeneratedXML = outXML.str();

	SnapshotDiff.SetOldProcesesList(vecProcesses);
	struct sockaddr_in Server_Address;
	int Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket == -1) {
		printf("Can Not Create A Socket!");
	}
	int Port = atoi(conf.ServerPort().c_str());
	Server_Address.sin_family = AF_INET;
	Server_Address.sin_port = htons(Port);
	Server_Address.sin_addr.s_addr = inet_addr(conf.ServerURL().c_str());
	if (Server_Address.sin_addr.s_addr == INADDR_NONE) {
		printf("Bad Address!");
	}
	connect(Socket, (struct sockaddr *) &Server_Address, sizeof(Server_Address));
	send(Socket, strGeneratedXML.c_str(), strGeneratedXML.length(), 0);
	close(Socket);
}

void SendData(CConfigMgr &conf, std::vector<CProcess>& vecProcesses,
		CSnapShotDiff& SnapshotDiff, bool bLatestSnaphot) {

	if (!bLatestSnaphot) {
		SendFullSnapShot(conf, vecProcesses, SnapshotDiff);
		printf("SendFullSnapShot");
	} else {
		SendDiffSnapshot(conf, vecProcesses, SnapshotDiff);
		printf("SendDiffSnapshot");
	}
}

int main() {

	signal(SIGPIPE, SIG_IGN);

	FILE* file = fopen("/proc/stat", "r");
	fscanf(file, "cpu %Ld %Ld %Ld %Ld", &lastTotalUser, &lastTotalUserLow,
			&lastTotalSys, &lastTotalIdle);
	fclose(file);

	CConfigMgr conf;
	CSnapShotDiff SnapshotDiff;
	bool bLatestSnaphot = false;

	std::vector<CProcess> vecProcesses;

	try {
		CProcessInfo psinfo;

		while (true) {
			sleep(conf.Interval());
			psinfo.CollectProcesses(vecProcesses);
			SendData(conf, vecProcesses, SnapshotDiff, bLatestSnaphot);
			bLatestSnaphot = true;
		}

	} catch (Util::CExceptionBase ex) {
		LOG(ex.Message().c_str());
	}

	LOG("Agent finishing...");
	return 0;
}

