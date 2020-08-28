/////////////////////////////////////////////////////////////////////////////
// Name:        W32Console.h
// Project:     scLib
// Purpose:     Console support classes & data types for Win32
// Author:      Piotr Likus
// Modified by:
// Created:     30/12/2010
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <boost/static_assert.hpp>

#include "sc/W32Console.h"
#include "sc/log.h"

// ----------------------------------------------------------------------------
// Local declarations
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// W32ConsoleManagerForShutdown
// ----------------------------------------------------------------------------
class W32ConsoleManagerForShutdown: public W32ConsoleManager {
public:
    W32ConsoleManagerForShutdown(scEventsQueue *msgTarget);
    virtual ~W32ConsoleManagerForShutdown();
protected:
    virtual bool intHandleCtrlEvent(const scDataNode &msg);
    virtual bool intHandleEvent(const scDataNode &hwnd, const scDataNode &msg, const scDataNode &wParam, const scDataNode &lParam, scDataNode &output);
    void signalEventAndWait(const scString &eventName);
protected:
    scEventsQueue *m_msgTarget;
};

// ----------------------------------------------------------------------------
// Local implementations
// ----------------------------------------------------------------------------
WNDPROC glConsoleWindowProc = NULL;

LRESULT CALLBACK LocConsoleWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    scDataNode output;
    bool res = 
        W32ConsoleManager::handleEvent(
          scDataNode(static_cast<void_ptr>(&hwnd)), 
          scDataNode(static_cast<ulong64>(msg)), 
          scDataNode(static_cast<ulong64>(wparam)), 
          scDataNode(static_cast<ulong64>(lparam)),
          output
        ); 

   if (res) {
       //if (!output.isNull()) {
       //    return output.getAsUInt64();
       //} else {
       //    return 0;
       //}
       return TRUE;
   } else {
       return CallWindowProc(glConsoleWindowProc, hwnd, msg, wparam, lparam);
   }
}

BOOL LocCtrlHandler(DWORD dwCtrlType)
{
    scDataNode output;
    BOOL res = FALSE;

    switch(dwCtrlType) {
      case CTRL_SHUTDOWN_EVENT:
      case CTRL_CLOSE_EVENT:
      case CTRL_LOGOFF_EVENT:   
        if (W32ConsoleManager::handleCtrlEvent(
          scDataNode(static_cast<ulong64>(dwCtrlType))
        )) {
          res = TRUE;
        }
        break;
      default:
        res = FALSE;
    }

    return res;
}

// ----------------------------------------------------------------------------
// W32ConsoleManager
// ----------------------------------------------------------------------------
W32ConsoleManager* W32ConsoleManager::m_activeObject = SC_NULL;

W32ConsoleManager::W32ConsoleManager(): W32ConsoleManagerIntf()
{
  if (m_activeObject == SC_NULL)
    m_activeObject = this; 
  m_closeEnabled = true;
  m_visible = true;
}
    
W32ConsoleManager::~W32ConsoleManager()
{
  if (m_activeObject == this)
    m_activeObject = SC_NULL;
  unregisterHandler();
}

void W32ConsoleManager::setVisible(bool value)
{
    intSetVisible(value);
    m_visible = value;
}

bool W32ConsoleManager::getVisible()
{
    return m_visible;
}

void W32ConsoleManager::setCloseEnabled(bool value)
{
    intSetCloseEnabled(value);
    m_closeEnabled = value;
}

bool W32ConsoleManager::getCloseEnabled()
{
    return m_closeEnabled;
}

W32ConsoleManager *W32ConsoleManager::checkActiveObject()
{
    if (m_activeObject == SC_NULL)
        throw scError("No active object for console manager");
    return m_activeObject;
}

void W32ConsoleManager::init()
{
    registerHandler();
}

bool W32ConsoleManager::handleEvent(const scDataNode &hwnd, const scDataNode &msg, const scDataNode &wParam, const scDataNode &lParam, scDataNode &output)
{
    return checkActiveObject()->intHandleEvent(hwnd, msg, wParam, lParam, output);
}

bool W32ConsoleManager::intHandleEvent(const scDataNode &hwnd, const scDataNode &msg, const scDataNode &wParam, const scDataNode &lParam, scDataNode &output)
{
    if (
         (msg.getAsUInt64() == WM_CLOSE) 
         &&
         (!checkActiveObject()->getCloseEnabled())
       ) 
    {
      output = scDataNode(0);
      return true;
    } else {
      return false;
    }
}

bool W32ConsoleManager::handleCtrlEvent(const scDataNode &msg)
{
    return checkActiveObject()->intHandleCtrlEvent(msg);
}

bool W32ConsoleManager::intHandleCtrlEvent(const scDataNode &msg)
{
    return false;
}

void W32ConsoleManager::intSetCloseEnabled(bool value)
{ // nothing required
}

void W32ConsoleManager::intSetVisible(bool value)
{
    if (!value)
        ShowWindow( GetConsoleWindow(), SW_HIDE );
    else
        ShowWindow( GetConsoleWindow(), SW_SHOWNORMAL );
}

void W32ConsoleManager::registerHandler()
{
    registerCtrlHandler();
}

void W32ConsoleManager::registerCtrlHandler()
{
    SetLastError(0);
    
    SetConsoleCtrlHandler(
      (PHANDLER_ROUTINE) LocCtrlHandler, // handler function
      TRUE); // add to list

    DWORD errorCd = GetLastError();
    if (errorCd != 0) {
        LPVOID lpMsgBuf;

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

        throw scError(msg);
    }

    // this is required since Vista & Win7 terminate interactive console applications instead of signaling logoff/shutdown/close
    //FreeConsole();
}

void W32ConsoleManager::registerWinHandler()
{
    if (glConsoleWindowProc == NULL) {
      SetLastError(0);
      glConsoleWindowProc = (WNDPROC)SetWindowLong(GetConsoleWindow(), GWL_WNDPROC, (LONG)LocConsoleWndProc);
      DWORD errorCd = GetLastError();

      if (errorCd != 0) {
          LPVOID lpMsgBuf;

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

          throw scError(msg);
      }
    }
}

void W32ConsoleManager::unregisterHandler()
{
    if (glConsoleWindowProc != NULL)
      SetWindowLong(GetConsoleWindow(), GWL_WNDPROC, (LONG)glConsoleWindowProc);

    SetConsoleCtrlHandler(
      (PHANDLER_ROUTINE) LocCtrlHandler, // handler function
      FALSE); // remove
}

// ----------------------------------------------------------------------------
// W32ConsoleManagerForShutdown
// ----------------------------------------------------------------------------
W32ConsoleManagerForShutdown::W32ConsoleManagerForShutdown(scEventsQueue *msgTarget): m_msgTarget(msgTarget)
{
    assert(msgTarget != SC_NULL);
}

W32ConsoleManagerForShutdown::~W32ConsoleManagerForShutdown()
{
}

bool W32ConsoleManagerForShutdown::intHandleCtrlEvent(const scDataNode &msg)
{
    bool res = false;

    switch (msg.getAsUInt64()) {
    case CTRL_CLOSE_EVENT:
        if (getCloseEnabled()) {
          signalEventAndWait("osnote.close_exec");
          res = false;
        } else {
          res = true;
        }
        break;
    case CTRL_SHUTDOWN_EVENT:
        signalEventAndWait("osnote.shutdown_exec");
        res = false;
        break;
    case CTRL_LOGOFF_EVENT:
        signalEventAndWait("osnote.logoff_exec");
        res = false;
        break;
    case CTRL_C_EVENT:
        signalEventAndWait("osnote.ctrl_c_exec");
        res = false;
        break;
    default:
        signalEventAndWait(scString("osnote.?.")+msg.getAsString());
        res = false;
    }

    return res;
}

bool W32ConsoleManagerForShutdown::intHandleEvent(const scDataNode &hwnd, const scDataNode &msg, 
    const scDataNode &wParam, const scDataNode &lParam, scDataNode &output)
{
    bool res = false;

    switch (msg.getAsUInt64()) {
    case WM_CLOSE:
        if (getCloseEnabled())
          signalEventAndWait("osnote.close_exec");
        output = scDataNode(0);
        res = true;
        break;
    case WM_ENDSESSION:
        signalEventAndWait("osnote.end_session_exec");
        output = scDataNode(0);
        res = true;
        break;
    default:
        res = false;
    }

    return res;
}

void W32ConsoleManagerForShutdown::signalEventAndWait(const scString &eventName)
{ 
    scLog::addInfo(scString("Signal [")+eventName+"] arrived");
    scLog::addDebug(scString("Signal [")+eventName+"] arrived");
    m_msgTarget->put(scEventsQueue::pack(eventName));
    while(!m_msgTarget->empty()) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    scLog::addDebug(scString("Signal [")+eventName+"] handled");
}

// ----------------------------------------------------------------------------
// W32ConsoleShutdownMonitor
// ----------------------------------------------------------------------------
W32ConsoleShutdownMonitor::W32ConsoleShutdownMonitor(): m_notifier(SC_NULL)
{
}

W32ConsoleShutdownMonitor::~W32ConsoleShutdownMonitor()
{
    m_manager.reset();
}

void W32ConsoleShutdownMonitor::init()
{
    m_queue.reset(newQueue());
    m_manager.reset(newConsoleManager());
    m_manager->init();
}

void W32ConsoleShutdownMonitor::setNotifier(scNotifier *value)
{
    m_notifier = value;
}

W32ConsoleManager *W32ConsoleShutdownMonitor::getConsoleManager()
{
    return m_manager.get();
}

void W32ConsoleShutdownMonitor::unload()
{
    assert(m_notifier != SC_NULL);
    m_queue->unloadTo(*m_notifier);
}

W32ConsoleManager *W32ConsoleShutdownMonitor::newConsoleManager()
{
    return new W32ConsoleManagerForShutdown(m_queue.get());
}

scEventsQueue *W32ConsoleShutdownMonitor::newQueue()
{
    return new scEventsQueueSync();
}
