/*
 * ConfigMgr.cpp
 *
 *  Created on: Aug 31, 2017
 *      Author: admin
 */

#include "ConfigMgr.h"
#include "ConfigMgrPimpl.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <libxml/tree.h>

CConfigMgr::CConfigMgr() :
	m_CPU(false), m_Mem(false), m_Read(false), m_Write(false),
	m_SysCPU(false), m_SysMem(false), m_SysRead(false), m_SysWrite(false), m_Interval(0) {

	m_pImpl = new CConfigMgrPimpl(this);

	m_AppConfig.sPort = "8093";
	m_AppConfig.sConfigFileName = "SnapshotParams.config";
	m_AppConfig.sServerPort = "8095";
	ReadAppConfig();
	Refresh();
}

CConfigMgr::~CConfigMgr() {
	delete m_pImpl;
}

std::string extract_str_value(const char* buffer) {
	std::string retval;

	char *begin = strstr((char*) buffer, "value=\"");
	if (begin) {
		char *end = strstr(begin + strlen("value=\""), "\"");
		if (end && (end > begin + strlen("value=\""))) {
			char value[500];
			memset(value, 0, 500);
			strncpy(value, begin + strlen("value=\""), end - begin - strlen(
					"value=\""));
			retval = value;
		}
	}
	return retval;
}

void CConfigMgr::ReadAppConfig() {
	FILE * conf = fopen("app.config", "r");
	if (conf) {
		char buffer[1024];

		do {
			if (fgets(buffer, sizeof(buffer), conf)) {
				if (strstr(buffer, "<!--"))
					continue;

				if (strstr(buffer, "PSServiceURI")) {
					m_AppConfig.sServiceURL = extract_str_value(buffer);
				} else if (strstr(buffer, "PSServerURL")) {
					m_AppConfig.sServerURL = extract_str_value(buffer);
				} else if (strstr(buffer, "DefaultFullSnapshotTime")) {
					std::string sTime = extract_str_value(buffer);
					sscanf(sTime.c_str(), "%d:%d", &m_AppConfig.nHH,
							&m_AppConfig.nMM);
				} else if (strstr(buffer, "RemoteControlPort")) {
					m_AppConfig.sPort = extract_str_value(buffer);
				} else if (strstr(buffer, "RemoteControlConfigFileName")) {
					m_AppConfig.sConfigFileName = extract_str_value(buffer);
				} else if (strstr(buffer, "SnapshotListenerPort")) {
					m_AppConfig.sServerPort = extract_str_value(buffer);
				}
			}
		} while (!feof(conf));
	}
}

void CConfigMgr::Refresh() {

		m_CPU = false;
		m_Mem = false;
		m_Read = false;
		m_Write = false;

		m_SysCPU = false;
		m_SysMem = false;
		m_SysRead = false;
		m_SysWrite = false;

	    xmlDoc *conf = NULL;
	    xmlNodePtr main_node = NULL;
	    xmlNodePtr child_node = NULL;

	    const char *Filename = this->ConfigFileName().c_str();

	    conf = xmlReadFile(Filename, NULL, 0);

	    if (conf != NULL)
	    {
	    	main_node = xmlDocGetRootElement(conf);

	        for (main_node = main_node->xmlChildrenNode; main_node; main_node = main_node->next)
	        {
	            if (main_node->type == XML_ELEMENT_NODE)
	            {
	            	if ((!xmlStrcmp(BAD_CAST main_node->name,BAD_CAST "SnapshotInterval")))
	            	{
	            		   m_Interval =  strtol( (const char *) xmlGetProp(main_node, BAD_CAST "value"), NULL,0);
	            	}
	            	else if( !xmlStrcmp(BAD_CAST main_node->name,BAD_CAST "ProcessMetrics"))
	                {
	                	 for (child_node = main_node->xmlChildrenNode; child_node; child_node = child_node->next)
	                	 {
	                		 std::cout << child_node->name << std::endl;
	        	        	 if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST "CPU"))){
									m_CPU = true;
	        	        	 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST "VirtualMemorySize"))) {
	        						m_Mem = true;
	        				 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST "ReadOperations"))) {
	        						m_Read = true;
	        				 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST "WriteOperations"))) {
	        						m_Write = true;
	        				 }
	                	 }
	                }
	            	else if( !xmlStrcmp(BAD_CAST main_node->name,BAD_CAST "SystemMetrics"))
	                {
	                	 for (child_node = main_node->xmlChildrenNode; child_node; child_node = child_node->next)
	                	 {
	                		 if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST  "ProcessorCPU"))) {
	                			 m_SysCPU = true;
	                		 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST  "UsedVirtualMemory"))) {
	                		 	 m_SysMem = true;
	                		 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST  "IOWriteDelta"))) {
	                		 	 m_SysWrite = true;
	                		 } else if ((!xmlStrcmp(BAD_CAST child_node->name,BAD_CAST  "IOReadDelta"))) {
	                		 	 m_SysRead = true;
	                		 }
	                	 }
	                }
	            }
	        }
	    }
	    else
	    {
	    	 std::cout << "error: could not parse file " << Filename << std::endl;
	    }
	    xmlCleanupParser();
}
