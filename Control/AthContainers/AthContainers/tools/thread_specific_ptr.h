// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/thread_specific_ptr.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Definition for thread_specific_ptr.
 *
 * In a standard build, we define the locking objects below.
 * In a standalone build, or with ATHCONTAINERS_NO_THREADS defined,
 * they're defined as no-ops.
 *
 * Split out from threading.h to reduce header dependencies.
 */


#ifndef ATHCONTAINERS_THREAD_SPECIFIC_PTR_H
#define ATHCONTAINERS_THREAD_SPECIFIC_PTR_H


#ifdef ATHCONTAINERS_NO_THREADS


namespace AthContainers_detail {


/**
 * @brief Single-thread version of thread_specific_ptr.
 */
template <class T>
class thread_specific_ptr
{
public:
  thread_specific_ptr() : m_ptr(0) {}
  ~thread_specific_ptr() { delete m_ptr; }
  thread_specific_ptr (const thread_specific_ptr&) = delete;
  thread_specific_ptr& operator= (const thread_specific_ptr&) = delete;
  T* get() { return m_ptr; }
  T* operator->() { return m_ptr; }
  T& operator*() { return *m_ptr; }
  const T* get() const { return m_ptr; }
  const T* operator->() const { return m_ptr; }
  const T& operator*() const { return *m_ptr; }
  void reset (T* new_value=0) { delete m_ptr; m_ptr = new_value; }
  T* release() { T* ret = m_ptr; m_ptr = 0; return ret; }

private:
  T* m_ptr;
};


} // namespace AthContainers_detail


#else  // not ATHCONTAINERS_NO_THREADS


#include "boost/thread/tss.hpp"


namespace AthContainers_detail {


using boost::thread_specific_ptr;


} // namespace AthContainers_detail



#endif // not ATHCONTAINERS_NO_THREADS


#endif // not ATHCONTAINERS_THREAD_SPECIFIC_PTR_H
