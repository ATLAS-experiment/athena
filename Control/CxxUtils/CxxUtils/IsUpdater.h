// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/IsUpdater.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2023
 * @brief Concept check for Updater class used by concurrent classes.
 */


#ifndef CXXUTILS_ISUPDATER_H
#define CXXUTILS_ISUPDATER_H


#include "CxxUtils/concepts.h"
#include <memory>


namespace CxxUtils {
namespace detail {


/**
 * @brief Concept check for Updater class used by concurrent classes.
 *
 * In order to implement updating concurrently with reading, we need to
 * defer deletion of objects until no thread can be referencing them any more.
 * The policy for this for the internal implementation objects
 * is set by the template UPDATER<T>.  An object
 * of this type owns an object of type T.  It should provide a typedef
 * Context_t, giving a type for an event context, identifying which
 * thread/slot is currently executing.  It should implement these operations:
 *
 *   - const T& get() const
 *     Return the current object.
 *   - void update (std::unique_ptr<T> p, const Context_t& ctx);
 *     Atomically update the current object to be p.
 *     Deletion of the previous version should be deferred until
 *     no thread can be referencing it.
 *   - void quiescent (const Context_t& ctx);
 *     Declare that the thread described by ctx is no longer accessing
 *     the object.
 *   - static const Context_t& defaultContext();
 *     Return a context object for the currently-executing thread.
 *
 * For an example, see AthenaKernel/RCUUpdater.h.
 */
template <template <class>  class UPDATER>
concept IsUpdater = requires(UPDATER<int> x,
                             const typename UPDATER<int>::Context_t& ctx)
{
  typename UPDATER<int>::Context_t;
  { x.get() } -> std::same_as<const int&>;
  { x.update (std::make_unique<int>(0), ctx) };
  { x.quiescent (ctx) };
  { x.defaultContext() } -> std::convertible_to<typename UPDATER<int>::Context_t>;
};


} // namespace detail
} // namespace CxxUtils


#endif // not CXXUTILS_ISUPDATER_H
