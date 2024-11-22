/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IDC_WRITEHANDLEBASE_H
#define IDC_WRITEHANDLEBASE_H
#include <atomic>

namespace EventContainers {


class IDC_WriteHandleBase{
protected:
   std::atomic<const void*>*  m_atomic;

   IDC_WriteHandleBase() : m_atomic(nullptr)
   { }
public:

   void LockOn(std::atomic<const void*>* in) noexcept {
      m_atomic = in;
   }
   void DropLock() noexcept;
   void ReleaseLock();

   ~IDC_WriteHandleBase();
};

}
#endif

