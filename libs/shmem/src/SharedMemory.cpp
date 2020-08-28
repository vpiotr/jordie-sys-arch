/////////////////////////////////////////////////////////////////////////////
// Name:        SharedMemory.cpp
// Project:     scLib
// Purpose:     Shared memory support classes
// Author:      Piotr Likus
// Modified by:
// Created:     12/01/2009
/////////////////////////////////////////////////////////////////////////////

#pragma message( "Compiling " __FILE__ ) 

#ifdef WIN32
#define SCSHM_WINDOWS
#endif

//sc
#include "sc/proc/SharedMemory.h"

#include "boost/interprocess/mapped_region.hpp"

#ifdef SCSHM_WINDOWS
#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/windows_shared_memory.hpp>
#endif

#ifdef SCSHM_WINDOWS
typedef boost::interprocess::windows_shared_memory scSharedMemObject;
#else
typedef boost::interprocess::shared_memory_object scSharedMemObject;
#endif
typedef boost::interprocess::mapped_region scSharedMemRegion;

scSharedMemory::scSharedMemory(const scString &a_path, scsmAccessMode accessMode, uint a_useFlags, 
  size_t a_size): 
  scSharedResource()
{
  using namespace boost::interprocess;
  
  m_size = a_size;
  m_path = a_path;
  m_accessMode = accessMode;
  m_ownsResource = (a_useFlags & scsmOwner);
  m_regionHandle = m_objectHandle = SC_NULL;
    
  bool createResource = ((a_useFlags & scsmCreate) != 0);  
  bool noAccess = ((a_useFlags & scsmNoAccess) != 0);
  
  if (createResource)
    freeResource();
  
  if (accessMode == scsmReadOnly) {
    if (createResource) {
    //== create + read-only
      m_objectHandle = new scSharedMemObject
       (open_or_create               //open or create
       ,stringToCharPtr(a_path)               //name
       ,read_only                    //read-only mode
#ifdef SCSHM_WINDOWS       
       ,m_size
#endif
       );      
    } else { // do not create
    //== no-create + read-only
      m_objectHandle = new scSharedMemObject
       (open_only                   //open-only
       ,stringToCharPtr(a_path)              //name
       ,read_only                   //read-only mode
       );      
    }
  } else { // read-write
    if (createResource) {
      m_objectHandle = new scSharedMemObject
       (open_or_create                  //open or create
       ,stringToCharPtr(a_path)               //name
       ,read_write                   //read-write mode
#ifdef SCSHM_WINDOWS       
       ,m_size
#endif       
       );      
    //== create + read-write
    } else {
    //== no-create + read-write
      m_objectHandle = new scSharedMemObject
       (open_only                    //open-only
       ,stringToCharPtr(a_path)               //name
       ,read_write                   //read-write mode
       );      
    }
  }
  
  if (!noAccess)
  {
    if (accessMode == scsmReadOnly)
      m_regionHandle = new scSharedMemRegion(*((scSharedMemObject *)m_objectHandle), read_only);
    else  
      m_regionHandle = new scSharedMemRegion(*((scSharedMemObject *)m_objectHandle), read_write);  
      
#ifndef SCSHM_WINDOWS
    if (createResource && (m_size>0))  
      ((scSharedMemObject *)m_objectHandle)->truncate(m_size);  
#endif      
  }
}

scSharedMemory::~scSharedMemory()
{
  if (m_ownsResource)
    freeResource();
  freeHandles();  
}

void scSharedMemory::freeHandles()
{
  delete ((scSharedMemRegion *)m_regionHandle);    
  m_regionHandle = SC_NULL;
  delete ((scSharedMemObject *)m_objectHandle);    
  m_objectHandle = SC_NULL;
}

scString scSharedMemory::getKeyName()
{
  return scString("shm:")+m_path;
}  

void scSharedMemory::freeResource()
{
  freeHandles();
#ifndef SCSHM_WINDOWS
  if (m_path.length() > 0)
    scSharedMemObject::remove(m_path.c_str());  
#endif
}

void *scSharedMemory::getAddress()
{
  void *res;
  if (m_regionHandle != SC_NULL)
  {
    res = (*((scSharedMemRegion *)m_regionHandle)).get_address();
  } else {
    res = SC_NULL;
  }
  return res;  
}

size_t scSharedMemory::getSize()
{
  return m_size;
}
