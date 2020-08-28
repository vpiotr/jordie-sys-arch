/////////////////////////////////////////////////////////////////////////////
// Name:        SharedMemory.h
// Project:     scLib
// Purpose:     Class for shared memory support
// Author:      Piotr Likus
// Modified by:
// Created:     11/01/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCSHAREDMEM_H__
#define _SCSHAREDMEM_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file SharedMemory.h
///
/// \brief Class for shared memory support
///
/// Usage:
/// \code
///     // Server which prepares memory block
///     //-----------------------------------------------
///     scString blockName = "test";
///
///     sharedGuard.reset(new scSharedMemory(blockName, 
///       scsmReadWrite, scsmOwner | scsmCreate, output.GetSize()));
///
///     // copy data to memory block
///     output.CopyTo(sharedGuard->getAddress(), output.GetSize());
///
///     // keep memory block until "free" msg arives from client
///     scSharedResourceManager::add(sharedGuard.release());
///
///     // Client which reads memory block
///     //-----------------------------------------------
///     string memoryBlockName = "test";
///     std::auto_ptr<scSharedMemory> shMemoryGuard;
///
///     shMemoryGuard.reset(new scSharedMemory(
///        memoryBlockName, 
///        scsmReadOnly, 0, blockSize));
///
///     void *mem = (char*)shMemoryGuard->getAddress();
///
///     // usage of mem:
///     ppm.Write(mem, blockSize);
///    
///     //---> notify server that we have used the memory block
/// \endcode



// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/proc/SharedResource.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
enum scsmAccessMode {
  scsmReadOnly,
  scsmReadWrite
};

enum scsmUseFlag {
  scsmOwner = 1,
  scsmCreate = 2,
  scsmNoAccess = 4,  // there will be no access to block
};

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------

class scSharedMemory: public scSharedResource {
public:  
  scSharedMemory(const scString &a_path, scsmAccessMode accessMode, uint a_useFlags, size_t a_size = 0);
  virtual ~scSharedMemory();
  virtual scString getKeyName();
  void *getAddress();
  size_t getSize();
protected:
  virtual void freeResource();  
  void freeHandles();  
protected:
  void *m_objectHandle;  
  void *m_regionHandle;  
  scString m_path;
  scsmAccessMode m_accessMode;
  bool m_ownsResource;
  size_t m_size;
};


#endif // _SCSHAREDMEM_H__
