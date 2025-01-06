// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/upgrade_mutex.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Definition for update_mutex.
 *
 * In a standard build, we define the locking objects below.
 * In a standalone build, or with ATHCONTAINERS_NO_THREADS defined,
 * they're defined as no-ops.
 *
 * Split out from threading.h to reduce header dependencies.
 */


#ifndef ATHCONTAINERS_UPGRADE_MUTEX_H
#define ATHCONTAINERS_UPGRADE_MUTEX_H


#ifdef ATHCONTAINERS_NO_THREADS


namespace AthContainers_detail {


/**
 * @brief No-op definition of @c upgrade_mutex.
 */
class upgrade_mutex
{
public:
  void lock() {}
  void unlock() {}
  void lock_shared() {}
  void unlock_shared() {}
  void lock_upgrade() {}
  void unlock_upgrade() {}
  void unlock_upgrade_and_lock() {}
private:
  // Real upgrade_mutex is not assignable.  Need this for proper
  // dictionary generation.
  upgrade_mutex& operator= (const upgrade_mutex&);
};


} // AthContainers_detail


#else  // not ATHCONTAINERS_NO_THREADS


#include "boost/thread/shared_mutex.hpp"


namespace AthContainers_detail {


using boost::upgrade_mutex;


} // namespace AthContainers_detail


#endif // not ATHCONTAINERS_NO_THREADS


#endif // not ATHCONTAINERS_UPGRADE_MUTEX_H
