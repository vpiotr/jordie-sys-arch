/////////////////////////////////////////////////////////////////////////////
// Name:        process.cpp
// Project:     scLib
// Purpose:     Process-related functions. OS-dependent.
// Author:      Piotr Likus
// Modified by:
// Created:     08/08/2009
/////////////////////////////////////////////////////////////////////////////

#include "sc/proc/process.h"

#define SLOW_PROCESSING

#ifdef WIN32
#include <windows.h>
#include "sc/proc/W32Process.h"
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#define UNIX_PROC_PRIORITY_BACKGROUD 5
#include "sc/dtypes.h"
#endif

//wx
#include <wx/utils.h> 

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

namespace proc { 

void setProcessPriorityAsBackgroud()
{
#ifdef SLOW_PROCESSING
#ifdef WIN32
SetPriorityClass (
                  GetCurrentProcess(),
                  BELOW_NORMAL_PRIORITY_CLASS
                  );
#else
    int err = setpriority(PRIO_PROCESS, 0, UNIX_PROC_PRIORITY_BACKGROUD);

		if( err != 0 )
			throw scError("Error, setpriority failed !");                  
#endif			
#endif
}

void sleepProcess(uint msecs)
{
  wxMilliSleep(msecs);
}

void sleepThisThreadMs(uint msecs)
{
  wxMilliSleep(msecs);
}

void sleepThisThreadUs(uint microSecs)
{
  wxMicroSleep(microSecs);
}

scProcessId getCurrentProcessId()
{
  return wxGetProcessId(); // wx
}  

scProcessId getParentProcessId(scProcessId processId)
{
  return W32_proc::GetParentProcessId(processId);
}

scProcessId getParentProcessId()
{
  return W32_proc::GetParentProcessId(GetCurrentProcessId());
}

bool processExists(scProcessId processId)
{
  return W32_proc::ProcessExists(processId);
}

void closeProcess(scProcessId processId)
{
  W32_proc::PostCloseApp(processId);
}

bool terminateProcess(scProcessId processId, unsigned long a_timeout)
{
  return ((W32_proc::TerminateAppShort(processId, a_timeout) & TA_SUCCESS_ANY) != 0);
}

void closeProcessByExec(const scString &execPath, bool excludeCurrent)
{
  W32_proc::CloseProcessByExec(const_cast<char *>(execPath.c_str()), excludeCurrent);
}

bool terminateProcessByExec(const scString &execPath, bool excludeCurrent, unsigned long a_timeout)
{
  return W32_proc::TerminateAppByExec(const_cast<char *>(execPath.c_str()), excludeCurrent, a_timeout);
}

unsigned int countProcessByExec(const scString &execPath, bool excludeCurrent)
{
  return W32_proc::CountProcessByExec(const_cast<char *>(execPath.c_str()), excludeCurrent);
}

unsigned int enumProcessByExec(const scString &execPath, bool excludeCurrent, scProcessEnumerator *enumProc)
{
  return W32_proc::EnumProcessByExec(const_cast<char *>(execPath.c_str()), excludeCurrent, enumProc);
}

}; // namespace proc
