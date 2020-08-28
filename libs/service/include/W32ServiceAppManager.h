/////////////////////////////////////////////////////////////////////////////
// Name:        W32ServiceAppManager.h
// Project:     scLib
// Purpose:     Win32 service manager for ServiceApp
// Author:      Piotr Likus
// Modified by:
// Created:     03/01/2011
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCW32SERVAPPMAN_H__
#define _SCW32SERVAPPMAN_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file W32ServiceAppManager.h
///
/// File description

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/ServiceApp.h"
#include "sc/proc/W32Service.h"

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

class scW32ServiceAppManager: public scW32ServiceManager {
public:
    scW32ServiceAppManager(scServiceApp *app): m_app(app), scW32ServiceManager() {}
    virtual ~scW32ServiceAppManager() {}
protected:
    virtual bool intPerformStep() {
        return m_app->performStep();
    }
    virtual bool intPrepare() {
        m_app->init();
        return true;
    }
    virtual void intStop() {
        m_app->stop();
    }
protected:
    scServiceApp *m_app;
};

#endif // _SCW32SERVAPPMAN_H__
