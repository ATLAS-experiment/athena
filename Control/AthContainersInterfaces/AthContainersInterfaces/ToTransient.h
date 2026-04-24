// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainersInterfaces/ToTransient.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2025
 * @brief Define a hook for running code after an object has been read.
 */


#ifndef ATHCONTAINERSINTERFACES_TOTRANSIENT_H
#define ATHCONTAINERSINTERFACES_TOTRANSIENT_H


class EventContext;


namespace SG {


/**
 * @brief Define a hook for running code after an object has been read.
 *
 * Some objects need additional processing after reading in order to be usable.
 * (ROOT read rules are often not ideal since there is no way to pass
 * them an EventContext other than via the global thread-local lookup.)
 * ToTransient<U>::toTransient will be called for the container of
 * an xAOD auxiliary variable after it has been read (for an auxiliary
 * variable of type T, this will usually be std::vector<T>).
 * It will also be called from ATLAS POOL converters after an object
 * has been read.
 *
 * If noToTransient is defined, then the converters may skip
 * actually making the toTransient calls for this type.
 */
template <class T>
class ToTransient
{
public:
  /// This definition flags taht we can skip actually calling @c toTransient
  /// for this type.  It should be aliased to @c int.
  using noToTransient = int;


  /*
   * @brief Do post-read processing for object @c o.
   * @param o The object that was just read.
   *
   * The default implementation is a no-op.
   */
  static void toTransient (T& /*o*/)
  {
  }


  /*
   * @brief Do post-read processing for object @c o.
   * @param o The object that was just read.
   * @param ctx The current event context.
   *
   * The default implementation is a no-op.
   */
  static void toTransient (T& /*o*/, const EventContext& /*ctx*/)
  {
  }
};


template <class T, class U>
constexpr
bool noToTransient1 (U) { return false; }
template <class T>
constexpr
bool noToTransient1 (int, typename T::noToTransient = typename T::noToTransient()) { return true; }


/**
 * Helper to test if T::noToTransient exists as a typedef.
 */
template <class T>
constexpr
bool noToTransient() { return noToTransient1<ToTransient<T> > (123); }


} // namespace SG


#endif // not ATHCONTAINERSINTERFACES_TOTRANSIENT_H
