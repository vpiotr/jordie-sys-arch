/////////////////////////////////////////////////////////////////////////////
// Name:        StartProcessThread.cpp
// Project:     scLib
// Purpose:     Thread used to start process in background.
// Author:      Piotr Likus
// Modified by:
// Created:     21/05/2013
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "sc/proc/StartProcessThread.h"

static size_t gs_counter = (size_t)-1;
static wxCriticalSection gs_critsect;
static wxSemaphore gs_cond;

scStartProcessThread::scStartProcessThread(const scString& command, const scString &params, bool a_minimized, bool a_lowPriority):
    m_command(command), 
    m_params(params), 
    m_minimized(a_minimized),
    m_lowPriority(a_lowPriority)
{
    Create();
}


wxThread::ExitCode scStartProcessThread::Entry()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	std::string cmd = m_command.c_str();
	std::string params = m_params.c_str();
	DWORD createParams;

  {
      wxCriticalSectionLocker lock(gs_critsect);
      if ( gs_counter == (size_t)-1 )
          gs_counter = 1;
      else
          gs_counter++;
  }
    	
	memset(& si, 0, sizeof(si));
	memset(& pi, 0, sizeof(pi));

  if (m_minimized)
  {
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    createParams = 0;
  } else {
    si.dwFlags = STARTF_USESHOWWINDOW;// | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_SHOW;
    createParams = CREATE_NEW_CONSOLE;
  }

  si.cb = sizeof(si);
  
  scString cmdLine = cmd + " " + params;
	BOOL started = CreateProcess(const_cast<char *>(cmd.c_str()), const_cast<char *>(cmdLine.c_str()), 
	  NULL, NULL, FALSE, createParams, 0, NULL, 
	  &si, &pi);
	
  if (m_lowPriority && started)
  {
    SetPriorityClass (
      pi.hProcess,
      BELOW_NORMAL_PRIORITY_CLASS
    );
  }  
	
  return 0;  
}

void scStartProcessThread::OnExit()
{
    wxCriticalSectionLocker lock(gs_critsect);
    if ( !--gs_counter )
        gs_cond.Post();
}

/// Wait until all threads terminate
void scStartProcessThread::WaitForAll()
{
  gs_cond.Wait();
}


