/*
 * ProcessInfo.h
 *
 *  Created on: Aug 8, 2012
 *      Author: admin
 */

#ifndef PROCESSINFO_H_
#define PROCESSINFO_H_

#include <vector>
#include "Process.h"

class CProcessInfo {

public:
	CProcessInfo();
	virtual ~CProcessInfo();

	void CollectProcesses(std::vector<CProcess> &vecProcesses);
};

#endif /* PROCESSINFO_H_ */
