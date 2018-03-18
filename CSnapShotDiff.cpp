#include <sstream>

#include "CSnapShotDiff.h"

long long CSnapShotDiff::CPUDelta = 0;
long long CSnapShotDiff::MEMDelta = 0;
long long CSnapShotDiff::IOWriteDelta = 0;
long long CSnapShotDiff::IOReadDelta = 0;

CSnapShotDiff::CSnapShotDiff() {
	// TODO Auto-generated constructor stub

}

CSnapShotDiff::~CSnapShotDiff() {
	// TODO Auto-generated destructor stub
}

void CSnapShotDiff::SetOldProcesesList(vector<CProcess>& vecProcesses) {
	int nCurrentProcListSize = vecProcesses.size();

	_vecOldProcesses.clear();

	for (int i = 0; i < nCurrentProcListSize; i++)
		_vecOldProcesses.push_back(vecProcesses[i]);
}

void CSnapShotDiff::GetDiffs(std::vector<CProcess> &vecProcesses) {
	std::string strConnectionType;
	std::string strLocalAddr;
	int nLocalPortXML;
	std::string strRemoteAddr;
	int nRemotePortXML;

	std::string strOldLocalAddr;
	int nOldLocalPortXML;
	std::string strOldRemoteAddr;
	int nOldRemotePortXML;

	_newDiffProcesses.clear();

	CProcess::IOReadCurrent = 0;
	CProcess::IOReadDelta = 0;
	CProcess::IOWriteCurrent = 0;
	CProcess::IOWriteDelta = 0;

	for (std::vector<CProcess>::iterator it = vecProcesses.begin(); it
			!= vecProcesses.end(); it++)
	{
		string strProcName = it->GetProcessName();
		int nCPU = it->GetCPU();
		int nMem = it->GetMem();
		unsigned long r, w;
		it->GetReadWriteBytes(r, w);
		CProcess::IOReadCurrent += r;
		CProcess::IOWriteCurrent += w;
		for (std::vector<CProcess>::iterator it_old = _vecOldProcesses.begin(); it_old
				!= _vecOldProcesses.end(); it_old++)
		{
			unsigned long old_r, old_w;
			it_old->GetReadWriteBytes(old_r, old_w);

			if (strProcName == it_old->GetProcessName()) {
				if (nCPU != it_old->GetCPU() || nMem != it_old->GetMem() || r
						!= old_r || w != old_w) {
					AddDiffComponent(*it);
					continue;

				}
			}

			for (std::vector<CConnectionCollector::SConnection>::iterator ps =
					it->m_vecConnections.begin(); ps
					!= it->m_vecConnections.end(); ps++) {

				ps->print(strConnectionType, strLocalAddr, nLocalPortXML,
						strRemoteAddr, nRemotePortXML);

				for (std::vector<CConnectionCollector::SConnection>::iterator
						ps_old = it_old->m_vecConnections.begin(); ps_old
						!= it_old->m_vecConnections.end(); ps_old++) {
					ps_old->print(strConnectionType, strOldLocalAddr,
							nOldLocalPortXML, strOldRemoteAddr,
							nOldRemotePortXML);

					if (strLocalAddr != strOldLocalAddr || nOldLocalPortXML
							!= nLocalPortXML || strRemoteAddr
							!= strOldRemoteAddr || nRemotePortXML
							!= nOldRemotePortXML) {
						AddDiffComponent(*it);
						break;
					}
				}
				break;
			} // for (std::vector<CConnectionCollector::SConnection>
		}
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

}

string CSnapShotDiff::CreateDiffXML(CConfigMgr &conf, std::vector<CProcess> &vecProcesses) {
	std::string strGeneratedXML;
	std::stringstream outXML;

	std::string strConnectionType;
	std::string strLocalAddr;
	int nLocalPortXML;
	std::string strRemoteAddr;
	int nRemotePortXML;

	GetDiffs(vecProcesses);

	for (std::vector<CProcess>::iterator it = _newDiffProcesses.begin(); it
			!= _newDiffProcesses.end(); it++) {
		unsigned long r, w;
		it->GetReadWriteBytes(r, w);

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

	strGeneratedXML = outXML.str();

	return strGeneratedXML;
}

void CSnapShotDiff::AddDiffComponent(CProcess& Process) {
	_newDiffProcesses.push_back(Process);
}
