/////////////////////////////////////////////////////////////////////////////
// Name:        W32Console.h
// Project:     scLib
// Purpose:     Console support classes & data types for Win32
// Author:      Piotr Likus
// Modified by:
// Created:     30/12/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _W32CONSOLE_H__
#define _W32CONSOLE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file core.h
///
/// File description

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"
#include "sc/events/EventsSync.h"

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
class W32ConsoleManagerIntf {
public:
    W32ConsoleManagerIntf() {};
    virtual ~W32ConsoleManagerIntf() {};
    virtual void setVisible(bool value) = 0;
    virtual bool getVisible() = 0;
    virtual void setCloseEnabled(bool value) = 0;
    virtual bool getCloseEnabled() = 0;
    virtual void init() = 0;
};

class W32ConsoleManager: public W32ConsoleManagerIntf {
public:
    W32ConsoleManager();
    virtual ~W32ConsoleManager();
    virtual void setVisible(bool value);
    virtual bool getVisible();
    virtual void setCloseEnabled(bool value);
    virtual bool getCloseEnabled();
    static bool handleEvent(const scDataNode &hwnd, const scDataNode &msg, const scDataNode &wParam, const scDataNode &lParam, scDataNode &output);
    static bool handleCtrlEvent(const scDataNode &msg);
    virtual void init();
protected:
    static W32ConsoleManager *checkActiveObject();
    virtual void intSetCloseEnabled(bool value);
    virtual void intSetVisible(bool value);
    virtual void registerHandler();
    virtual void unregisterHandler();
    virtual void registerCtrlHandler();
    virtual void registerWinHandler();
    virtual bool intHandleEvent(const scDataNode &hwnd, const scDataNode &msg, const scDataNode &wParam, const scDataNode &lParam, scDataNode &output);
    virtual bool intHandleCtrlEvent(const scDataNode &msg);
protected:
    static W32ConsoleManager *m_activeObject;
    bool m_closeEnabled;
    bool m_visible;
};

class W32ConsoleShutdownMonitor {
public:
    W32ConsoleShutdownMonitor();
    virtual ~W32ConsoleShutdownMonitor();
    virtual void init();
    void setNotifier(scNotifier *value);
    void unload();
    W32ConsoleManager *getConsoleManager();
protected:
    virtual W32ConsoleManager *newConsoleManager();
    virtual scEventsQueue *newQueue();
protected:
    std::auto_ptr<W32ConsoleManager> m_manager;
    std::auto_ptr<scEventsQueue> m_queue;
    scNotifier *m_notifier;
};

// ----------------------------------------------------------------------------
// Global namespace functions
// ----------------------------------------------------------------------------

#endif // _W32CONSOLE_H__
