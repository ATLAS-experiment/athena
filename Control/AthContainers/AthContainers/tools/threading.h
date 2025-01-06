// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/tools/threading.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Threading definitions.
 *
 * In a standard build, we define the locking objects below.
 * In a standalone build, or with ATHCONTAINERS_NO_THREADS defined,
 * they're defined as no-ops.
 */


#ifndef ATHCONTAINERS_THREADING_H
#define ATHCONTAINERS_THREADING_H


#ifdef ATHCONTAINERS_NO_THREADS


namespace AthContainers_detail {


/**
 * @brief No-op definition of @c mutex.
 */
class mutex {};


/**
 * @brief No-op definition of @c recursive_mutex.
 */
class recursive_mutex {};


/**
 * @brief No-op definition of @c lock_guard.
 */
template <class LOCKABLE>
class lock_guard
{
public:
  lock_guard() {}
  lock_guard(LOCKABLE&) {}
};


/// No-op definitions of fences.
inline void fence_acq_rel() {}
inline void fence_seq_cst() {}


/// Dummy version of atomic.
template <class T>
class atomic
{
public:
  atomic() : m_val() {}
  operator T() const { return m_val; }
  atomic& operator= (const T& val) { m_val = val; return *this; }

private:
  T m_val;
};


} // namespace AthContainers_detail


#else  // not ATHCONTAINERS_NO_THREADS

//the shared_mutex include generates annoying unused_variable warnings during compilations... silence this in athanalysisbase
#ifdef XAOD_ANALYSIS
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED 1
#endif
#endif


#include <atomic>
#include <mutex>
#include <thread>


namespace AthContainers_detail {


using std::mutex;
using std::recursive_mutex;
using std::lock_guard;
using std::thread;
using std::atomic;


/**
 * @brief An acquire/release fence.
 */
void fence_acq_rel();


/**
 * @brief A sequentially-consistent fence.
 */
void fence_seq_cst();


} // namespace AthContainers_detail


#endif // not ATHCONTAINERS_NO_THREADS



namespace AthContainers_detail {


/**
 * @brief Lock object for taking out shared locks.
 *
 * This is like the boost @c strict_lock, except that it takes out a shared
 * lock on the underlying object rather than an exclusive one.
 */
template <typename LOCKABLE>
class strict_shared_lock
{
public:
  /// The underlying object type.
  typedef LOCKABLE lockable_type;


  /**
   * @brief Take out a shared lock on @c obj and remember it.
   * @param obj The lockable object.
   */
  explicit strict_shared_lock(lockable_type& obj);


  /**
   * @brief Take out a shared lock on @c obj and remember it.
   * @param obj The lockable object.
   */
  explicit strict_shared_lock(const lockable_type& obj);


  /**
   * @brief Release the held lock.
   */
  ~strict_shared_lock();


private:
  // Disallow these.
  strict_shared_lock();
  strict_shared_lock(strict_shared_lock const&);
  strict_shared_lock& operator=(strict_shared_lock const&);


private:
  /// The lock being held.
  lockable_type& m_obj;
};


/**
 * @brief Lock object for taking out upgradable locks.
 *
 * The will first be taken out in upgrade mode.
 * If @c upgrade() is called, then the lock will be upgraded to exclusive mode.
 * When this object is destroyed, the object will be unlocked for either
 * upgrade or exclusive mode, as appropriate.
 */
template <class LOCKABLE>
class upgrading_lock
{
public:
  /// The underlying object type.
  typedef LOCKABLE lockable_type;


  /**
   * @brief Take out an upgrade lock on @c obj and remember it.
   * @param obj The lockable object.
   */
  explicit upgrading_lock (lockable_type& obj);


  /**
   * @brief Release the held lock.
   */
  ~upgrading_lock();


  /**
   * @brief Convert the lock from upgrade to exclusive.
   */
  void upgrade();


private:
  // Disallow these.
  upgrading_lock();
  upgrading_lock(upgrading_lock const&);
  upgrading_lock& operator=(upgrading_lock const&);


private:
  /// The lock being held.
  lockable_type& m_obj;

  /// Has the lock been converted to exclusive?
  bool m_exclusive;
};


} // namespace AthContainers_detail


#include "AthContainers/tools/threading.icc"


#endif // not ATHCONTAINERS_THREADING_H
