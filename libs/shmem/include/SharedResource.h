/////////////////////////////////////////////////////////////////////////////
// Name:        SharedResource.h
// Project:     scLib
// Purpose:     Shared resource support classes
// Author:      Piotr Likus
// Modified by:
// Created:     11/01/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCSHAREDRES_H__
#define _SCSHAREDRES_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file SharedResource.h
///
/// Shared memory manager, keeps global list of smart references to shared
/// memory blocks. On last "remove reference" action shared memory is freed.
/// 
/// Usage:
/// \code
///     std::auto_ptr<scSharedMemory> sharedGuard;
///     // fill data...
///
///     // keep memory block until "free" msg arives from client
///     scSharedResourceManager::add(sharedGuard.release(), "test");
///     
///     // release reference to memory block:
///     scSharedResourceManager::releaseRef("test");
/// \endcode

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

//std
#include <map>
// boost
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include "boost/ptr_container/ptr_list.hpp"
#include "boost/ptr_container/ptr_map.hpp"
// sc
#include "sc/dtypes.h"
#include "base/object.h"

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

/// implements resource that can be referenced or destroyed on demand
class scSharedResource: public scReferenceCounter {
public:
  scSharedResource() {};
  virtual ~scSharedResource() {};
  virtual void freeResource() = 0;
  virtual scString getKeyName() = 0;
};

typedef boost::intrusive_ptr<scSharedResource> scSharedResourceTransporter;
typedef boost::ptr_map<scString,scSharedResourceTransporter> scResourceMapColn;
typedef std::multimap<scString,bool> scResourceMMapColn;

/// controls destruction of resource
class scSharedResourceManager: public scObject {
public:
  scSharedResourceManager();
  virtual ~scSharedResourceManager();
  static void add(scSharedResource *a_resource, const scString &keyName = scString(""));
  static void addRef(const scString &keyName);
  static void releaseRef(const scString &keyName);
  static bool ready();
  static scSharedResource *get(const scString &keyName);
  static scSharedResource *find(const scString &keyName);
protected:
  static scSharedResourceManager *checkManager();
  void intAdd(scSharedResource *a_resource, const scString &keyName = scString(""));
  void intAddRef(const scString &keyName = scString(""));
  void intReleaseRef(const scString &keyName);
  scSharedResource *intGet(const scString &keyName);
  scSharedResource *intFind(const scString &keyName);
private:
  static scSharedResourceManager *m_activeManager;    
  scResourceMapColn m_resourceList;
  scResourceMMapColn m_handleList;
};

#endif // _SCSHAREDRES_H__
