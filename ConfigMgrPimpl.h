/*
 * ConfigMgrPimpl.h
 *
 *  Created on: Sep 2, 2017
 *      Author: admin
 */

#ifndef CONFIGMGRPIMPL_H_
#define CONFIGMGRPIMPL_H_

#include <pthread.h>
class CConfigMgr;

class CConfigMgrPimpl {
	pthread_t m_thread_id;
	CConfigMgr* m_pMgr;
public:
	CConfigMgrPimpl(CConfigMgr* pMgr);
	virtual ~CConfigMgrPimpl();

	void new_connection(int fd);

private:
	void send_config(int fd);
	void recv_config(int fd);

	int sendall(int s, char *buf, int *len);
	static void* ListenerThread(void*);
};

#endif /* CONFIGMGRPIMPL_H_ */
