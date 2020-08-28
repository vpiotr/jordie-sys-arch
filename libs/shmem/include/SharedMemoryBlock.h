/////////////////////////////////////////////////////////////////////////////
// Name:        SharedMemoryBlock.h
// Project:     scLib
// Purpose:     Handles I/O for shared memory
// Author:      Piotr Likus
// Modified by:
// Created:     12/02/2011
/////////////////////////////////////////////////////////////////////////////


#ifndef _SCSHMEMBLCK_H__
#define _SCSHMEMBLCK_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file SharedMemoryBlock.h
\brief Handles I/O for shared memory

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc\dtypes.h"

#include "sc/proc/SharedMemory.h"

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
class scShmWinConsumerIntf {
public:
  scShmWinConsumerIntf() {}
  virtual ~scShmWinConsumerIntf() {}
  virtual void process(const char *cptr, size_t size) = 0;
};

class scShmWinWriterIntf {
public:
  scShmWinWriterIntf() {}
  virtual ~scShmWinWriterIntf() {}
  virtual size_t write(char *output, size_t outputSize) = 0;
};

class scSharedMemoryBlock {
public:
  enum ShBlockAccessType { shbat_read_only, shbat_read_write, shbat_create };

  scSharedMemoryBlock(const scString &blockPath, size_t aSize);
  ~scSharedMemoryBlock();
  void create();
  bool exists();
  void registerBlock(ShBlockAccessType accessType, scSharedMemory *memory, bool owner);
  bool read(scShmWinConsumerIntf *consumer);
  bool read(scShmWinConsumerIntf *consumer, size_t aOffset, size_t aLimit);
  void write(scShmWinWriterIntf *writer, size_t aOffset, size_t aLimit);

  /// \brief Calculate length of data stored in block
  /// \param data pointer to beginning of block data 
  /// \return Returns length of data stored in a given block
  size_t length(const char *data, size_t dataSize);

  void clear();
  void clear(size_t aOffset, size_t aLimit);
  static void copy(const scString &blockPathSrc, const scString &blockPathDest, size_t blockSize, bool useReadOnly);
  static void copy(const scString &blockPathSrc, const scString &blockPathDest, size_t blockSize, size_t aOffset, size_t aLimit, bool useReadOnly);
protected:
  static size_t recalcLimit(size_t aBlockSize, size_t aOffset, size_t aLimit);
  size_t recalcLimit(size_t aOffset, size_t aLimit);
  void checkPos(size_t aOffset, size_t aLimit);
  void *get(ShBlockAccessType accessType);
  static void *get(const scString &path, ShBlockAccessType accessType);
  scString calcRegPath(ShBlockAccessType accessType);
  static scString calcRegPath(const scString &path, ShBlockAccessType accessType);
private:
  scString m_path;
  size_t m_size;
};


#endif // _SCSHMEMBLCK_H__
