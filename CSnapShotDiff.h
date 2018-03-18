
#ifndef CSNAPSHOTDIFF_H_
#define CSNAPSHOTDIFF_H_

#include <vector>
#include "Process.h"
#include "ConfigMgr.h"

using namespace std;

class CSnapShotDiff {
public:
	CSnapShotDiff();
	virtual ~CSnapShotDiff();

	void SetOldProcesesList(vector<CProcess>& vecProcesses);
	string CreateDiffXML(CConfigMgr &conf, std::vector<CProcess> &vecProcesses);

private:

	void GetDiffs(std::vector<CProcess> &vecProcesses);
	void AddDiffComponent(CProcess& Process);

	vector<CProcess> _vecOldProcesses;

	vector<CProcess> _newDiffProcesses;

	static long long CPUDelta;
	static long long MEMDelta;
	static long long IOWriteDelta;
	static long long IOReadDelta;
};

#endif /* CSNAPSHOTDIFF_H_ */
