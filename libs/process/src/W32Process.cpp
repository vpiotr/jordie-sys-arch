/////////////////////////////////////////////////////////////////////////////
// Name:        W32Process.cpp
// Project:     scLib
// Purpose:     Process-related functions. Win32 API.
// Author:      Piotr Likus
// Modified by:
// Created:     21/05/2013
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tlhelp32.h>

#include "sc/proc/W32Process.h"

namespace W32_proc { 
  
// ----------------------------------------------------------------------------
// system block
// ----------------------------------------------------------------------------

typedef struct
{
  DWORD   dwID ;
  DWORD   dwThread ;
} TERMINFO ;

// Declare Callback Enum Functions.
BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam );

/*----------------------------------------------------------------
   DWORD WINAPI TerminateAppShort( DWORD dwPID, DWORD dwTimeout )

   Purpose:
      Shut down a 32-Bit Process (or 16-bit process under Windows 95)

   Parameters:
      dwPID
         Process ID of the process to shut down.

      dwTimeout
         Wait time in milliseconds before shutting down the process.

   Return Value:
      TA_FAILED - If the shutdown failed.
      TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
      TA_SUCCESS_KILL - if the process was shut down with
         TerminateProcess().
      NOTE:  See header for these defines.
   ----------------------------------------------------------------*/ 
DWORD WINAPI TerminateAppShort( DWORD dwPID, DWORD dwTimeout )
{
  HANDLE   hProc ;
  DWORD   dwRet ;

  // If we can't open the process with PROCESS_TERMINATE rights,
  // then we give up immediately.
  hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE,
     dwPID);

  if(hProc == NULL)
  {
     return TA_FAILED ;
  }

  // Wait on the handle. If it signals, great. If it times out,
  // then you kill it.
  if(WaitForSingleObject(hProc, dwTimeout)!=WAIT_OBJECT_0)
     dwRet=(TerminateProcess(hProc,0)?TA_SUCCESS_KILL:TA_FAILED);
  else
     dwRet = TA_SUCCESS_CLEAN ;

  CloseHandle(hProc) ;

  return dwRet ;
}

/*----------------------------------------------------------------
   DWORD WINAPI PostCloseApp( DWORD dwPID )

   Purpose:
      Shut down a 32-Bit Process (or 16-bit process under Windows 95)

   Parameters:
      dwPID
         Process ID of the process to shut down.

      dwTimeout
         Wait time in milliseconds before shutting down the process.

   Return Value:
      TA_FAILED - If the shutdown failed.
      TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
      TA_SUCCESS_KILL - if the process was shut down with
         TerminateProcess().
      NOTE:  See header for these defines.
   ----------------------------------------------------------------*/ 
void WINAPI PostCloseApp( DWORD dwPID )
{
  // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
  // matches your process's.
  EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID) ;
}

BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam )
{
  DWORD dwID ;

  GetWindowThreadProcessId(hwnd, &dwID) ;

  if(dwID == (DWORD)lParam)
  {
     PostMessage(hwnd, WM_CLOSE, 0, 0) ;
  }

  return TRUE ;
}

/// Returns executive file path for the specified process
scString GetExePath(DWORD dwProcID)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,dwProcID);
	MODULEENTRY32 me;
	scString path;
	me.dwSize=sizeof(me);

	if (Module32First(hSnap,&me))
		path = me.szExePath;

	CloseHandle(hSnap);
	return path;
}

bool CloseProcessByExec(LPSTR szExeName, bool excludeCurrent)
{
  bool res = false;
  std::string path;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  DWORD currProcessId = GetProcessId(GetCurrentProcess());
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
        if (Pc.th32ProcessID != 0) 
        {
          path = W32_proc::GetExePath(Pc.th32ProcessID).c_str();
          if(!_stricmp(path.c_str(), szExeName)) {
            if (!excludeCurrent || (Pc.th32ProcessID != currProcessId))
            {
              W32_proc::PostCloseApp(Pc.th32ProcessID);
              res = true;
            }  
          }
        }
      }while(Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

bool TerminateAppByExec(LPSTR szExeName, bool excludeCurrent, unsigned int a_timeout)
{
  bool res = false;
  std::string path;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  DWORD currProcessId = GetProcessId(GetCurrentProcess());
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
        if (Pc.th32ProcessID != 0) 
        {
          path = W32_proc::GetExePath(Pc.th32ProcessID).c_str();
          if(!_stricmp(path.c_str(), szExeName)) {
            if (!excludeCurrent || (Pc.th32ProcessID != currProcessId))
            {
              W32_proc::TerminateAppShort(Pc.th32ProcessID, a_timeout);
              res = true;
            }  
          }
        }
      }while(Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

unsigned int CountProcessByExec(LPSTR szExeName, bool excludeCurrent)
{
  unsigned int res = 0;
  std::string path;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  DWORD currProcessId = GetProcessId(GetCurrentProcess());
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
        if (Pc.th32ProcessID != 0) 
        {
          path = W32_proc::GetExePath(Pc.th32ProcessID).c_str();
          if(!_stricmp(path.c_str(), szExeName)) {
            if (!excludeCurrent || (Pc.th32ProcessID != currProcessId))
            {
              ++res;
            }  
          }
        }
      }while(Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

unsigned int EnumProcessByExec(LPSTR szExeName, bool excludeCurrent, scProcessEnumerator *enumProc)
{
  unsigned int res = 0;
  std::string path;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  DWORD currProcessId = GetProcessId(GetCurrentProcess());
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
        if (Pc.th32ProcessID != 0) 
        {
          path = W32_proc::GetExePath(Pc.th32ProcessID).c_str();
          if(!_stricmp(path.c_str(), szExeName)) {
            if (!excludeCurrent || (Pc.th32ProcessID != currProcessId))
            {
              if (enumProc != SC_NULL) 
                (*enumProc)(Pc.th32ProcessID);
              ++res;
            }  
          }
        }
      }while(Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

unsigned int EnumProcesses(bool excludeCurrent, scProcessEnumerator *enumProc)
{
  unsigned int res = 0;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  DWORD currProcessId = GetProcessId(GetCurrentProcess());
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
        if (Pc.th32ProcessID != 0) 
        {
          if (!excludeCurrent || (Pc.th32ProcessID != currProcessId))
          {
            if (enumProc != SC_NULL) 
              (*enumProc)(Pc.th32ProcessID);
            ++res;
          }  
        }  
      }while(Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

unsigned long GetParentProcessId(unsigned long processId)
{
  unsigned long res = 0;
  PROCESSENTRY32 Pc = { sizeof(PROCESSENTRY32) };
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    if(Process32First(hSnapshot, &Pc)){
      do{
          if (Pc.th32ProcessID == processId)
            res = Pc.th32ParentProcessID;
      }while((res == 0) && Process32Next(hSnapshot, &Pc));
    }
    CloseHandle(hSnapshot);  
  }
  return res;
}

bool ProcessExists(unsigned long pid)
{
  class scW32CheckEnumerator: public scProcessEnumerator {
  public:
    scW32CheckEnumerator(DWORD searchPid): scProcessEnumerator() {m_searchPid = searchPid; m_found = false;};
    virtual ~scW32CheckEnumerator() {}  
    virtual void operator()(scProcessId pid) {if (pid == m_searchPid) m_found = true;};
    bool isProcessFound() {return m_found;}
  protected:
    DWORD m_searchPid;  
    bool m_found;
  };

  scW32CheckEnumerator enumer(pid);
  W32_proc::EnumProcesses(false, &enumer);
  return enumer.isProcessFound();
}

}; // namespace W32_proc
