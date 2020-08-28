/////////////////////////////////////////////////////////////////////////////
// Name:        StartProcessThread.h
// Project:     scLib
// Purpose:     Thread used to start process in background.
// Author:      Piotr Likus
// Modified by:
// Created:     21/05/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCSTRTPROCTHRD_H__
#define _SCSTRTPROCTHRD_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file StartProcessThread.h
\brief Thread used to start process in background.

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "wx/thread.h"

#include "sc/strings.h"

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
// Class definitions
// ----------------------------------------------------------------------------

class scStartProcessThread: public wxThread
{
public:
    scStartProcessThread(const scString& command, const scString &params, bool a_minimized, bool a_lowPriority); 
    virtual ExitCode Entry();
    virtual void OnExit();
    static void WaitForAll();
private:
    scString m_command;
    scString m_params;
    bool m_minimized;
    bool m_lowPriority;
};


#endif // _SCSTRTPROCTHRD_H__
