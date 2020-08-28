/////////////////////////////////////////////////////////////////////////////
// Name:        SharedMemoryBlock.cpp
// Project:     scLib
// Purpose:     Handles I/O for shared memory
// Author:      Piotr Likus
// Modified by:
// Created:     12/02/2011
/////////////////////////////////////////////////////////////////////////////

#include "sc/proc/SharedMemoryBlock.h"

#include <boost/interprocess/detail/win32_api.hpp>

#include "perf\Log.h"
#include "perf\Counter.h"

#include "sc/utils.h"

using namespace perf;

//#define DEBUG_SHM_CREATE

inline size_t shared_block_length(const char *data, size_t dataSize)
{
  //JSON: 
  //return std::char_traits<char>::length(data) + 1;
  //BION:
  if (dataSize < sizeof(size_t))
    return 0;
  else {
    size_t realSize = *reinterpret_cast<const size_t *>(data);
    return realSize;
  }
}

inline void shared_block_copy(void *dest, void *src, size_t dataSize)
{
  //JSON:
  //strncpy(dest, src, dataSize);
  //BION:
  size_t realSize = shared_block_length(reinterpret_cast<char *>(src), dataSize);
  realSize += sizeof(size_t);
  //Log::addDebug("Performing copy of "+toString(realSize)+" bytes");
  std::memcpy(dest, src, realSize);
}


class ShmWinWriterForMem: public scShmWinWriterIntf {
public:
  ShmWinWriterForMem(const char *input, size_t inputSize): 
      m_input(input), m_inputSize(inputSize), scShmWinWriterIntf() 
  {}

  size_t write(char *output, size_t outputSize)
  {
    size_t dataLimit = SC_MIN(outputSize, m_inputSize);
    std::memcpy(output, m_input, dataLimit);
    return dataLimit;
  }
private:
  const char *m_input;
  size_t m_inputSize;
};

scSharedMemoryBlock::scSharedMemoryBlock(const scString &blockPath, size_t aSize): m_path(blockPath), m_size(aSize)
{
}

scSharedMemoryBlock::~scSharedMemoryBlock()
{
}

size_t scSharedMemoryBlock::length(const char *data, size_t dataSize) 
{
  return shared_block_length(const_cast<char *>(data), dataSize);
}

void scSharedMemoryBlock::create()
{
  assert(m_size > 0);

#ifdef DEBUG_SHM_CREATE
  Log::addDebug(scString("creating shared block, size: ")+toString(m_size)+", path: ["+m_path+"]");
#endif

#ifdef TRACE_IO_CNT
  Counter::inc("io-total");
  Counter::inc("io-shm-block-create-cnt");
  Counter::inc("io-shm-block-create-size", m_size);
#endif

  std::auto_ptr<scSharedMemory>
    sharedGuard(new scSharedMemory(m_path, 
       scsmReadWrite, scsmOwner | scsmCreate, m_size));

  std::memset(sharedGuard->getAddress(), 0, m_size);

  registerBlock(shbat_read_write, sharedGuard.release(), true);
}

void scSharedMemoryBlock::registerBlock(scSharedMemoryBlock::ShBlockAccessType accessType, scSharedMemory *memory, bool owner)
{
  std::string regPath = calcRegPath(m_path, accessType);

  if (scSharedResourceManager::find(regPath) != NULL)
  {
    delete memory;
    throw std::runtime_error("Shared block already registered: "+regPath);
  }

#ifdef TRACE_IO_CNT
  Counter::inc("io-shm-block-reg-cnt");
  Counter::inc("io-shm-block-reg-size", memory->getSize());
#endif

  scSharedResourceManager::add(memory, regPath); 
}

scString scSharedMemoryBlock::calcRegPath(ShBlockAccessType accessType)
{
  return calcRegPath(m_path, accessType);
}

scString scSharedMemoryBlock::calcRegPath(const scString &path, ShBlockAccessType accessType)
{
  if (accessType == shbat_create)
    return path;
  else
    return path + "_" + ((accessType == shbat_read_only)?"rd":"wr");
}

bool scSharedMemoryBlock::exists()
{
  using namespace boost::interprocess;

  std::string regPath = calcRegPath(m_path, shbat_create);
  if (scSharedResourceManager::find(regPath) != NULL)
    return true;

  try {
     unsigned long file_map_access = winapi::page_readonly;
     unsigned long map_access = winapi::file_map_read;

     void *test_handle = winapi::open_file_mapping(map_access, m_path.c_str());

     bool res = (test_handle != NULL);

     if(res) 
        winapi::close_handle(test_handle);    

     return res;
  } 
  catch(...) {
    return false;
  }
}

bool scSharedMemoryBlock::read(scShmWinConsumerIntf *consumer)
{
   return read(consumer, 0, m_size);
}

bool scSharedMemoryBlock::read(scShmWinConsumerIntf *consumer, size_t aOffset, size_t aLimit)
{
  checkPos(aOffset, aLimit);

  void *data = get(shbat_read_only);
  if (data != NULL)
  {
    const char *mem = (char*)data;

    size_t realLimit = recalcLimit(aOffset, aLimit);
    assert(realLimit > 0);
    size_t sizeLimit = shared_block_length(mem+aOffset, realLimit);
    if (aLimit < sizeLimit)
      sizeLimit = aLimit;

    consumer->process(mem+aOffset+sizeof(size_t), sizeLimit);
  } else {

    std::auto_ptr<scSharedMemory> shMemoryGuard;

    shMemoryGuard.reset(new scSharedMemory(
      m_path, 
      scsmReadOnly, 0, m_size));

    const char *mem = (char*)shMemoryGuard->getAddress();

    size_t realLimit = recalcLimit(aOffset, aLimit);
    assert(realLimit > 0);
    size_t sizeLimit = shared_block_length(mem+aOffset, realLimit);
    if (aLimit < sizeLimit)
      sizeLimit = aLimit;

    consumer->process(mem+aOffset+sizeof(size_t), sizeLimit);

    registerBlock(shbat_read_only, shMemoryGuard.release(), false);
  }

  return true;
}

void scSharedMemoryBlock::write(scShmWinWriterIntf *writer, size_t aOffset, size_t aLimit)
{
  checkPos(aOffset, aLimit);
  size_t realLimit = recalcLimit(aOffset, aLimit);
  assert(realLimit > 0);

  void *shdata = get(shbat_read_write);
  if (shdata != NULL)
  {
    char *cptr = (char *)shdata;
    size_t bytesWritten;
    if (realLimit > sizeof(size_t)) 
      bytesWritten = writer->write(cptr+aOffset+sizeof(size_t), realLimit - sizeof(size_t));
    else
      bytesWritten = 0;
    std::memcpy(cptr+aOffset, &bytesWritten, sizeof(size_t));
    assert(shared_block_length(cptr+aOffset, sizeof(size_t)) == bytesWritten);
  } else {
    std::auto_ptr<scSharedMemory> sharedGuard(
        new scSharedMemory(m_path, scsmReadWrite, 0, m_size));

    char *cptr = (char *)sharedGuard->getAddress();
    size_t bytesWritten;
    if (realLimit > sizeof(size_t))
      bytesWritten = writer->write(cptr+aOffset+sizeof(size_t), realLimit - sizeof(size_t));
    else
      bytesWritten = 0;

    std::memcpy(cptr+aOffset, &bytesWritten, sizeof(size_t));
    assert(shared_block_length(cptr+aOffset, sizeof(size_t)) == bytesWritten);
    
    registerBlock(shbat_read_write, sharedGuard.release(), false);
  }
}

void scSharedMemoryBlock::clear()
{
  clear(0, m_size);
}

void scSharedMemoryBlock::clear(size_t aOffset, size_t aLimit)
{
  char clearChars[] = {'\0'};
  size_t clrSize = sizeof(clearChars) / sizeof(char);
  ShmWinWriterForMem writer(clearChars, clrSize);
  write(&writer, aOffset, clrSize);
}

void scSharedMemoryBlock::copy(const scString &blockPathSrc, const scString &blockPathDest, size_t blockSize, bool useReadOnly)
{
  copy(blockPathSrc, blockPathDest, blockSize, 0, blockSize, useReadOnly);
}

void scSharedMemoryBlock::copy(const scString &blockPathSrc, const scString &blockPathDest, size_t blockSize, size_t aOffset, size_t aLimit, bool useReadOnly)
{
  char *dataSrc;

  // read-write can be used to limit number of blocks being allocated
  if (useReadOnly)
    dataSrc = (char *)get(blockPathSrc, shbat_read_only);
  else
    dataSrc = (char *)get(blockPathSrc, shbat_read_write);

  char *dataDest = (char *)get(blockPathDest, shbat_read_write);
  size_t realLimit = recalcLimit(blockSize, aOffset, aLimit);

  if ((dataSrc != NULL) && (dataDest != NULL))
  {
    shared_block_copy(dataDest+aOffset, dataSrc+aOffset, realLimit);
  } else {
    std::auto_ptr<scSharedMemory> sharedGuardSrc(
      new scSharedMemory(blockPathSrc, scsmReadOnly, 0, blockSize));

    const char *memSrc = (char*)sharedGuardSrc->getAddress();

    std::auto_ptr<scSharedMemory> sharedGuardDest(
        new scSharedMemory(blockPathDest, scsmReadWrite, 0, blockSize));

    char *dptr = (char *)sharedGuardDest->getAddress(); 
    shared_block_copy(dptr+aOffset, const_cast<char *>(memSrc+aOffset), realLimit);

    if (dataSrc == NULL) {
      scSharedMemoryBlock block(blockPathSrc, blockSize);
      block.registerBlock(shbat_read_only, sharedGuardSrc.release(), false);
    }

    if (dataDest == NULL) {
      scSharedMemoryBlock block(blockPathDest, blockSize);
      block.registerBlock(shbat_read_write, sharedGuardDest.release(), false);
    }
  }
}

size_t scSharedMemoryBlock::recalcLimit(size_t aOffset, size_t aLimit)
{
    return recalcLimit(m_size, aOffset, aLimit);
}

size_t scSharedMemoryBlock::recalcLimit(size_t aBlockSize, size_t aOffset, size_t aLimit)
{
    size_t endPos = SC_MIN(aBlockSize, aOffset + aLimit);
    return (endPos >= aOffset)?endPos - aOffset:0;
}

void scSharedMemoryBlock::checkPos(size_t aOffset, size_t aLimit)
{
  if (aOffset + aLimit > m_size)
    throw std::runtime_error(
      scString("Shared block offset + limit incorrect")+
        ", size="+toString(m_size)+
        ", offset="+toString(aOffset)+
        ", limit="+toString(aLimit)+
        ", path=["+m_path+"]");
}

void *scSharedMemoryBlock::get(ShBlockAccessType accessType)
{
  return get(m_path, accessType);
}

void *scSharedMemoryBlock::get(const scString &path, ShBlockAccessType accessType)
{
#ifdef TRACE_IO_CNT
  Counter::inc("io-shm-block-get-cnt");
#endif

  std::string regPath = calcRegPath(path, accessType);
  scSharedMemory *memory = checked_cast<scSharedMemory *>(scSharedResourceManager::find(regPath));
  if (memory != NULL) {

#ifdef TRACE_IO_CNT
  Counter::inc("io-shm-block-get-hit-cnt");
#endif

    return memory->getAddress();
  } else {

#ifdef TRACE_IO_CNT
  Counter::inc("io-shm-block-get-mis-cnt");
#endif

    return NULL;
  }
}

