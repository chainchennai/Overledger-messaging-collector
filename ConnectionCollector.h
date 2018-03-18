/*
 * ConnectionCollector.h
 *
 *  Created on: Aug 30, 2012
 *      Author: admin
 */

#ifndef CONNECTIONCOLLECTOR_H_
#define CONNECTIONCOLLECTOR_H_

#include <map>
#include <string>
#include <iostream>
#include <vector>

class CConnectionCollector {
public:
	class SConnection {
	public:
		enum eConnectionType {
			eType_TCP = 0, eType_UDP,
		} eType;

		std::string sLocalAddr;
		int nLocalPort;
		std::string sRemoteAddr;
		int nRemotePort;

		void print(std::string& strConnectionType, std::string& strLocalAddr, int& nLocalPortXML,
		std::string& strRemoteAddr, int& nRemotePortXML) 
		{
			strConnectionType =  (eType == eType_TCP) ? "TCP" : "UDP";

			strLocalAddr = sLocalAddr;
			nLocalPortXML = nLocalPort;
			strRemoteAddr = sRemoteAddr;
			nRemotePortXML = nRemotePort;
		}

		SConnection() {
		}

		SConnection(const SConnection & sconn) {
			eType = sconn.eType;
			sLocalAddr = sconn.sLocalAddr;
			nLocalPort = sconn.nLocalPort;
			sRemoteAddr = sconn.sRemoteAddr;
			nRemotePort = sconn.nRemotePort;
		}

		SConnection(eConnectionType _eType, std::string _sLocalAddr,
				int _nLocalPort, std::string _sRemoteAddr, int _nRemotePort) :
			eType(_eType), sLocalAddr(_sLocalAddr), nLocalPort(_nLocalPort),
					sRemoteAddr(_sRemoteAddr), nRemotePort(_nRemotePort) {
		}
	};

public:
	CConnectionCollector();
	virtual ~CConnectionCollector();

	void Refresh();
	void GetConnectionsForProcessID(pid_t pid, std::vector<SConnection> &vecConnections);
private:
	std::map<unsigned long, SConnection> m_mapConnInode;

private:
	void ReadConnections(const char * file, SConnection::eConnectionType type, std::map<unsigned long, SConnection> &mapConnInode);
};

#endif /* CONNECTIONCOLLECTOR_H_ */
