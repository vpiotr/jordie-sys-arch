/////////////////////////////////////////////////////////////////////////////
// Name:        process.h
// Project:     scLib
// Purpose:     Process-related functions. OS-dependent.
// Author:      Piotr Likus
// Modified by:
// Created:     08/08/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCPROCESS_H__
#define _SCPROCESS_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file process.h
/// \brief Process-related functions. OS-dependent.
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
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
namespace proc {

void setProcessPriorityAsBackgroud();

void sleepProcess(uint msecs);
void sleepThisThreadMs(uint msecs);
void sleepThisThreadUs(uint microSecs);

scProcessId getCurrentProcessId();
scProcessId getParentProcessId(scProcessId processId);
scProcessId getParentProcessId();
bool processExists(scProcessId processId);

void closeProcess(scProcessId processId);
/// @param[in] a_timeout timeout in ms
bool terminateProcess(scProcessId processId, unsigned long a_timeout = 0);

void closeProcessByExec(const scString &execPath, bool excludeCurrent = true);

/// Terminate process selected by executable path
/// @param[in] execPath path to executable file
/// @param[in] excludeCurrent specifies if current process should be ignored or not
/// @param[in] a_timeout timeout in ms
/// @return Returns true on success
bool terminateProcessByExec(const scString &execPath, bool excludeCurrent = true, unsigned long a_timeout = 0);
unsigned int countProcessByExec(const scString &execPath, bool excludeCurrent = true);
unsigned int enumProcessByExec(const scString &execPath, bool excludeCurrent = true, scProcessEnumerator *enumProc = SC_NULL);

}; // namespace

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

#endif // _SCPROCESS_H__
