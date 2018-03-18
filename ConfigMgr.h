/*
 * ConfigMgr.h
 *
 *  Created on: Aug 31, 2012
 *      Author: admin
 */

#ifndef CONFIGMGR_H_
#define CONFIGMGR_H_

#include <string>
#include <libxml/parser.h>

class CConfigMgrPimpl;

class CConfigMgr {

	struct SAppConfig {
		std::string sServiceURL;
		std::string sServerURL;
		std::string sPort;
		std::string sConfigFileName;
		std::string sServerPort;
		int nHH;
		int nMM;
	} m_AppConfig;

	bool m_CPU;
	bool m_Mem;
	bool m_Read;
	bool m_Write;

	bool m_SysCPU;
	bool m_SysMem;
	bool m_SysRead;
	bool m_SysWrite;

	unsigned int m_Interval;
	CConfigMgrPimpl* m_pImpl;

public:
	CConfigMgr();
	virtual ~CConfigMgr();

	bool CPU() { return m_CPU; }
	bool Mem() { return m_Mem; }
	bool Read() { return m_Read; }
	bool Write() { return m_Write; }

	bool SysCPU() { return m_SysCPU; }
	bool SysMem() { return m_SysMem; }
	bool SysRead() { return m_SysRead; }
	bool SysWrite() { return m_SysWrite; }
	unsigned int Interval() { return m_Interval; }

	void ReadAppConfig();
	void Refresh();

	std::string ConfigFileName() { return m_AppConfig.sConfigFileName; }
	std::string Port() { return m_AppConfig.sPort; }
	std::string ServerPort() { return m_AppConfig.sServerPort; }
	std::string ServerURL() { return m_AppConfig.sServerURL; }
};

#endif /* CONFIGMGR_H_ */


