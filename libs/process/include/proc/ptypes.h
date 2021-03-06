/////////////////////////////////////////////////////////////////////////////
// Name:        ptypes.h
// Project:     scLib
// Purpose:     Process-related data types.
// Author:      Piotr Likus
// Modified by:
// Created:     21/05/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCPROCTYPES_H__
#define _SCPROCTYPES_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file ptypes.h
/// \brief Process-related data types.
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
typedef unsigned long scProcessId;

class scProcessEnumerator {
public:
  scProcessEnumerator() {};
  virtual ~scProcessEnumerator() {}  
  virtual void operator()(scProcessId pid) = 0;
};

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

#endif // _SCPROCTYPES_H__
