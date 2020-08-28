/////////////////////////////////////////////////////////////////////////////
// Name:        process.h
// Project:     scLib
// Purpose:     Process-related functions. OS-dependent.
// Author:      Piotr Likus
// Modified by:
// Created:     08/08/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCW32PROCESS_H__
#define _SCW32PROCESS_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file process.h
///
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <Windows.h>

#include "sc/dtypes.h"
#include "sc/proc/ptypes.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------
namespace W32_proc {

#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define TA_SUCCESS_ANY 3

unsigned long GetParentProcessId(unsigned long processId);
bool ProcessExists(unsigned long pid);

/// @param[in] a_timeout timeout in ms
DWORD WINAPI TerminateAppShort(DWORD dwPID, DWORD dwTimeout);
void WINAPI PostCloseApp(DWORD dwPID);
scString GetExePath(DWORD dwProcID);
bool CloseProcessByExec(LPSTR szExeName, bool excludeCurrent = true);
/// @param[in] a_timeout timeout in ms
bool TerminateAppByExec(LPSTR szExeName, bool excludeCurrent = true, unsigned int a_timeout = 0);
unsigned int CountProcessByExec(LPSTR szExeName, bool excludeCurrent = true);
unsigned int EnumProcessByExec(LPSTR szExeName, bool excludeCurrent = true, scProcessEnumerator *enumProc = SC_NULL);
unsigned int EnumProcesses(bool excludeCurrent = true, scProcessEnumerator *enumProc = SC_NULL);

}; // namespace W32_proc


// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

#endif // _SCW32PROCESS_H__
