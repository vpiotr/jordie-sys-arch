/////////////////////////////////////////////////////////////////////////////
// Name:        W32Service.cpp
// Project:     scLib
// Purpose:     Win32 service framework.
// Author:      Piotr Likus
// Modified by:
// Created:     31/12/2010
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
//sc
#include "sc/dtypes.h"
#include "sc/utils.h"
#include "perf/Log.h"

#include "sc/proc/W32Service.h"
#include "sc/proc/process.h"

using namespace perf;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE ServiceControlEvent = 0;

void RunService();
bool InstallService();
bool UninstallService();
scString getLastWin32ErrorText();

// ----------------------------------------------------------------------------
// Sample service implementation below
// Usage:
//   std::auto_ptr<scW32ServiceManager> service(new testService());
//   scW32ServiceManager::run();
// ----------------------------------------------------------------------------

//class testService: public scW32ServiceManager {
//public: 
//    testService(): scW32ServiceManager() {}
//    virtual ~testService() {}
//protected:
//    virtual scString intGetServiceKeyName() { return "httpbsvc"; }
//    virtual bool intPerformStep() {
//        sleepProcess(50);
//        return true;
//    }
//};


// ----------------------------------------------------------------------------
// Local variables
// ----------------------------------------------------------------------------
scW32ServiceManager* scW32ServiceManager::m_activeObject = SC_NULL;

// ----------------------------------------------------------------------------
// Local implementation
// ----------------------------------------------------------------------------

scW32ServiceManager::scW32ServiceManager()
{
    if (m_activeObject == SC_NULL)
        m_activeObject = this;
    m_state = wssNull;
}

scW32ServiceManager::~scW32ServiceManager()
{
    if (m_activeObject == this)
       m_activeObject = SC_NULL;
}

scString scW32ServiceManager::getServiceKeyName()
{
    return checkObject()->intGetServiceKeyName();
}

scString scW32ServiceManager::getServiceDispName()
{
    return checkObject()->intGetServiceDispName();
}

bool scW32ServiceManager::prepare()
{
    checkState(wssNull);
    if (checkObject()->intPrepare()) {
        setState(wssPrepared);
        return true;
    } else {
        return false;
    }
}

void scW32ServiceManager::unprepare()
{
    checkState(wssPrepared);
    checkObject()->intUnprepare();
    setState(wssNull);
}

bool scW32ServiceManager::start()
{
    checkState(wssPrepared);
    if (checkObject()->intStart()) {
        setState(wssStarted);
        return true;
    } else {
        return false;
    }
}

void scW32ServiceManager::stop()
{
    checkState(wssStarted);
    checkObject()->intStop();
    setState(wssPrepared);
}

bool scW32ServiceManager::performStep()
{
    checkState(wssStarted);
    return checkObject()->intPerformStep();
}

bool scW32ServiceManager::install()
{
    checkObject();
    return InstallService();
}

bool scW32ServiceManager::uninstall()
{
    checkObject();
    return UninstallService();
}

void scW32ServiceManager::run()
{
    checkObject();
    RunService();
}

scString scW32ServiceManager::intGetServiceDispName()
{
    return intGetServiceKeyName();
}

bool scW32ServiceManager::intPrepare()
{ //empty
    return true;
}

void scW32ServiceManager::intUnprepare()
{ //empty
}

bool scW32ServiceManager::intStart()
{ //empty
    return true;
}

void scW32ServiceManager::intStop()
{ //empty
}

void scW32ServiceManager::checkState(W32ServiceState value)
{
    if (checkObject()->m_state != static_cast<uint>(value))
        throw scError(scString("Invalid service state: [")+toString(checkObject()->m_state)+" != "+toString(value)+"]");
}

void scW32ServiceManager::setState(W32ServiceState value)
{
    checkObject()->m_state = value;
}

scW32ServiceManager *scW32ServiceManager::checkObject()
{
    if (m_activeObject == SC_NULL)
        throw scError("Service manager not available");
    return m_activeObject;
}

// ----------------------------------------------------------------------------
// Local Win32 code
// ----------------------------------------------------------------------------
void WINAPI ServiceControlHandler( DWORD controlCode )
{
	switch ( controlCode )
	{
		case SERVICE_CONTROL_INTERROGATE:
		break;

		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
    		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus( serviceStatusHandle, &serviceStatus );

			SetEvent( ServiceControlEvent );
			return;

		case SERVICE_CONTROL_PAUSE:
			break;

		case SERVICE_CONTROL_CONTINUE:
			break;

		default:
			if ( controlCode >= 128 && controlCode <= 255 )
			// user defined control code
				break;
			else
			// unrecognised control code
				break;
	}

	SetServiceStatus( serviceStatusHandle, &serviceStatus );
}

void WINAPI ServiceMain( DWORD /*argc*/, TCHAR* /*argv*/[] )
{
        Log::addDebug("ServiceMain - start");
	// initialise service status
	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	serviceStatus.dwControlsAccepted = 0;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

        const char *serviceKeyName = scW32ServiceManager::getServiceKeyName().c_str();
        serviceStatusHandle = RegisterServiceCtrlHandler( serviceKeyName, ServiceControlHandler );

	if ( serviceStatusHandle )
	{
		// service is starting
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// Create the Controlling Event here
 		ServiceControlEvent = CreateEvent( 0, FALSE, FALSE, 0 );

		// Service running
		serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );
		
		//////////////////////////////////////////////////////////////////////////////////////////////
                scW32ServiceManager::prepare();

                DWORD waitRes = 0;
                bool bStartOk = scW32ServiceManager::start();
		if (bStartOk)
                        Log::addInfo("Service started correctly");
                else
                        Log::addInfo("Service start error");

                if (bStartOk) {
                    bool bRunAgain = true;

		    do
		    {
                            try {
                                bRunAgain = scW32ServiceManager::performStep();
                            } 
                            catch(...) {
                                Log::addError(scString("Exception during performStep"));
                                bRunAgain = false;
                            }
			    waitRes = WaitForSingleObject( ServiceControlEvent, 30 );
		    }
		    while ( bRunAgain && (waitRes == WAIT_TIMEOUT ));
		    scW32ServiceManager::stop();
                }


                scW32ServiceManager::unprepare();
		///////////////////////////////////////////////////////////////////////////////////////////////
	
		// service was stopped
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// do cleanup here
		
		CloseHandle( ServiceControlEvent );
		ServiceControlEvent = 0;

		// service is now stopped
		serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );
	}
        Log::addDebug("ServiceMain - end");
}

void RunService()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
                { 0, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
		{ 0, 0 }
	};

        scString keyName = scW32ServiceManager::getServiceKeyName();

        serviceTable[0].lpServiceName = const_cast<char *>(keyName.c_str());
        SetLastError(0);

        StartServiceCtrlDispatcher( serviceTable );
        
        DWORD errorCd = GetLastError();

        if (errorCd != 0) {
          scString errTxt = getLastWin32ErrorText();
          throw scError(errTxt+" ("+toString(errorCd)+")");
        }
}

scString getLastWin32ErrorText()
{
    LPVOID lpMsgBuf;

    DWORD errorCd = GetLastError();

    FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorCd,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, NULL );

    scString msg((LPCTSTR)lpMsgBuf);

    LocalFree(lpMsgBuf);

    return msg;
}

bool InstallService()
{
        bool res = false;

        SetLastError(0);
	SC_HANDLE serviceControlManager = 
             OpenSCManager( 0, 0, SC_MANAGER_CREATE_SERVICE );

        scString keyName = scW32ServiceManager::getServiceKeyName();

	if ( serviceControlManager )
	{
 		char path[ _MAX_PATH + 1 ];
		if ( GetModuleFileName( 0, path, sizeof(path)/sizeof(path[0]) ) > 0 )
		{
			SC_HANDLE service = CreateService( 
                          serviceControlManager,
                          keyName.c_str(), 
                          stringToCharPtr(scW32ServiceManager::getServiceDispName()),
			  SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
			  SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
			  0, 0, 0, 0, 0 );
    		if ( service )
			{
                                res = true;
				CloseServiceHandle( service );
                                Log::addInfo(scString("Service installed successfully: ")+
                                    keyName);
			}
			else
			{
				DWORD dwError;
				dwError = GetLastError();
				if(dwError == ERROR_SERVICE_EXISTS)
                                        Log::addError(scString("Service already exists: ")+
                                            keyName);
				else
                                        Log::addError(scString("Unknown install error [")+
                                            toString(dwError)+"]: "+
                                            keyName);
			}
		}

		CloseServiceHandle( serviceControlManager );
	}
        return res;
}

bool UninstallService()
{
        bool res = false;

        SetLastError(0);

	SC_HANDLE serviceControlManager = OpenSCManager( 0, 0,
	SC_MANAGER_CONNECT );

        scString keyName = scW32ServiceManager::getServiceKeyName();

	if ( serviceControlManager )
	{
		SC_HANDLE service = OpenService( serviceControlManager,
                    keyName.c_str(), SERVICE_QUERY_STATUS | DELETE );

		if ( service )
		{
			SERVICE_STATUS serviceStatus;
			if ( QueryServiceStatus( service, &serviceStatus ) )
			{
				if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
				{
					if(DeleteService( service )) {
                                                res = true;
                                                Log::addInfo(scString("Service removed successfully: ")+
                                                   keyName);
                                        }
					else
					{
						DWORD dwError;
						dwError = GetLastError();
						if(dwError == ERROR_ACCESS_DENIED)
                                                    Log::addError(scString("Access denied while trying to remove: ")+
                                                        keyName);
						else if(dwError == ERROR_INVALID_HANDLE)
                                                    Log::addError(scString("Handle invalid while trying to remove: ")+
                                                        keyName);
						else if(dwError == ERROR_SERVICE_MARKED_FOR_DELETE)
                                                    Log::addError(scString("Service already marked for deletion: ")+
                                                        keyName);
                                                else 
                                                    Log::addError(scString("Unknown delete error [")+
                                                        toString(dwError)+"]: "+
                                                        keyName);
					}
				}
				else
				{
                                        Log::addError(scString("Service is still running: ")+
                                                        keyName);
				}
			}
			CloseServiceHandle( service );
		}
		CloseServiceHandle( serviceControlManager );
	}

        return res; 
}


