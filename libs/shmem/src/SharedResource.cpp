/////////////////////////////////////////////////////////////////////////////
// Name:        SharedResource.cpp
// Project:     scLib
// Purpose:     Shared resource support classes
// Author:      Piotr Likus
// Modified by:
// Created:     12/01/2009
/////////////////////////////////////////////////////////////////////////////

#include "sc/proc/SharedResource.h"
#include "sc/dtypes.h"
#include "sc/utils.h"
#include "perf/Timer.h"

// undefine to detect resource allocation problems on destroy
#define SRM_AUTO_FREE
// define to track release of resources
//#define SC_SHRES_TRACK 

using namespace perf;

// ----------------------------------------------------------------------------
// scSharedResourceManager
// ----------------------------------------------------------------------------
scSharedResourceManager* scSharedResourceManager::m_activeManager = SC_NULL;

scSharedResourceManager::scSharedResourceManager()
{
  if (m_activeManager != SC_NULL)
    throw scError("Resource manager already created!");
  m_activeManager = this;  
}

scSharedResourceManager::~scSharedResourceManager()
{
#ifdef TRACE_IO_TIME  
Timer::start("io-shm-res-man-destroy");
#endif  

#ifdef SC_SHRES_TRACK
  if (m_handleList.size()) {  
#ifndef SRM_AUTO_FREE  
    assert(m_handleList.size() == 0);
#endif    
  }  
  if (m_resourceList.size()) {
#ifndef SRM_AUTO_FREE  
    assert(m_resourceList.size() == 0);
#endif    
  }  
#endif  
  if (m_activeManager == this)
    m_activeManager = SC_NULL;
#ifdef TRACE_IO_TIME  
Timer::stop("io-shm-res-man-destroy");
#endif  
}

bool scSharedResourceManager::ready()
{
  return (m_activeManager != SC_NULL);
}

void scSharedResourceManager::add(scSharedResource *a_resource, const scString &keyName)
{
  checkManager()->intAdd(a_resource, keyName);
}

void scSharedResourceManager::addRef(const scString &keyName)
{
  checkManager()->intAddRef(keyName);
}

void scSharedResourceManager::releaseRef(const scString &keyName)
{
  checkManager()->intReleaseRef(keyName);
}

scSharedResource *scSharedResourceManager::get(const scString &keyName)
{
  return checkManager()->intGet(keyName);
}

scSharedResource *scSharedResourceManager::find(const scString &keyName)
{
  return checkManager()->intFind(keyName);
}

scSharedResourceManager *scSharedResourceManager::checkManager()
{
  if (m_activeManager == SC_NULL)
    throw scError("No resource manager found!");
    
  return m_activeManager;      
}

void scSharedResourceManager::intAdd(scSharedResource *a_resource, const scString &keyName)
{
  scString useName = keyName;
  if (useName.length() == 0)
    useName = a_resource->getKeyName();
    
  m_resourceList.insert(useName, new scSharedResourceTransporter(a_resource));
  intAddRef(useName);

#ifdef SC_SHRES_TRACK     
      scLog::addInfo("Resource registered: "+useName);                           
#endif      
}

void scSharedResourceManager::intAddRef(const scString &keyName)
{
  m_handleList.insert(std::pair<scString, bool>(keyName,true));
}

void scSharedResourceManager::intReleaseRef(const scString &keyName)
{
#ifdef SC_SHRES_TRACK     
      scLog::addInfo("Releasing reference to: "+keyName);                           
#endif      
  scResourceMMapColn::iterator fi, pback;
  scResourceMapColn::iterator resi;
  
  fi = m_handleList.find(keyName);
  if(fi != m_handleList.end()) { // found a name
    fi = m_handleList.erase(fi);
    if (fi == m_handleList.upper_bound(keyName))
    { // last one - free resource    
      resi = m_resourceList.find(keyName);
      if (resi != m_resourceList.end())
        m_resourceList.erase(resi);
#ifdef SC_SHRES_TRACK     
      scLog::addInfo("Resource released");                           
#endif      
    }
  }
}

scSharedResource *scSharedResourceManager::intGet(const scString &keyName)
{
  scSharedResource *res = intFind(keyName);
  if (res == NULL)
    throw scError(scString("Resource not found: ")+keyName);
  return res;
}

scSharedResource *scSharedResourceManager::intFind(const scString &keyName)
{
  scResourceMapColn::iterator resi;
  resi = m_resourceList.find(keyName);
  if (resi == m_resourceList.end())
    return NULL;
  else
    return resi->second->get();
}
