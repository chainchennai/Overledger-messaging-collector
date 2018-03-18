/*
 * Exceptions.h
 *
 *  Created on: Aug 8, 2017
 *      Author: admin
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>

namespace Util
{

	class CExceptionBase
	{
	protected:
		std::string msg;
	public:
		CExceptionBase() : msg("") {}
		virtual ~CExceptionBase() {}
		virtual std::string Message() { return msg; }
	};

	class CExceptionProcfsNotFound : public CExceptionBase
	{
	public:
		CExceptionProcfsNotFound(std::string procfs)
		{
			msg = "proc filesystem not mounted at " + procfs;
		}
	};

}

#endif /* EXCEPTIONS_H_ */
