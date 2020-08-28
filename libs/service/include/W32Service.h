/////////////////////////////////////////////////////////////////////////////
// Name:        W32Service.h
// Project:     scLib
// Purpose:     Win32 service framework.
// Author:      Piotr Likus
// Modified by:
// Created:     31/12/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCW32SERVICE_H__
#define _SCW32SERVICE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file W32Service.h
///
/// File description

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
enum W32ServiceState {
    wssNull,
    wssPrepared,
    wssStarted
};

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

class scW32ServiceManager {
public:
    scW32ServiceManager();
    virtual ~scW32ServiceManager();
    static scString getServiceKeyName();
    static scString getServiceDispName();
    static bool prepare();
    static void unprepare();
    static bool start();
    static void stop();
    static bool performStep();
    static bool install();
    static bool uninstall();
    static void run();
protected:
    virtual scString intGetServiceKeyName() = 0;
    virtual scString intGetServiceDispName();
    virtual bool intPrepare();
    virtual void intUnprepare();
    virtual bool intStart();
    virtual void intStop();
    virtual bool intPerformStep() = 0;
    static void checkState(W32ServiceState value);
    static void setState(W32ServiceState value);
    static scW32ServiceManager *checkObject();
protected:
    static scW32ServiceManager *m_activeObject;
    uint m_state;
};


#endif // _SCW32SERVICE_H__
