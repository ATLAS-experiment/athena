// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/RefCountedPtr.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Simple smart pointer for Gaudi-style refcounted objects.
 */


#ifndef CXXUTILS_REFCOUNTEDPTR_H
#define CXXUTILS_REFCOUNTEDPTR_H


#include "CxxUtils/concepts.h"
#include <memory>  // For unique_ptr


namespace CxxUtils {


/**
 * @brief Simple smart pointer for Gaudi-style refcounted objects.
 *
 * Smart pointer maintaining reference counts on an object.
 * The type @c T must have @c addRef and @c release methods.
 *
 * This is basically the Gaudi @c SmartIF class with the @c IInterface
 * dependency removed.
 *
 * Further note: accessors are not const, and copy/assignment methods
 * take non-const objects.  This is to avoid warnings from the thread-safety
 * checker.
 */
template <CxxUtils::detail::RefCounted T>
class RefCountedPtr
{
public:
  using element_type = T;

  /// Default constructor.
  RefCountedPtr() = default;

  /// Constructor from pointer.  Adds to the refcount.
  RefCountedPtr (T* p);

  /// Constructor from unique_ptr.  Adds to the refcount.
  template <CxxUtils::detail::RefCounted OTHER>
  requires std::convertible_to<OTHER*, T*>
  RefCountedPtr (std::unique_ptr<OTHER>&& other);

  /// Copy constructor.
  RefCountedPtr (const RefCountedPtr& other);

  /// Move constructor.
  RefCountedPtr (RefCountedPtr&& other);

  /// Constructor from another RefCountedPtr, with a different type.
  template <CxxUtils::detail::RefCounted OTHER>
  requires std::convertible_to<OTHER*, T*>
  RefCountedPtr (const RefCountedPtr<OTHER>& other);

  /// Move from another RefCountedPtr, with a different type.
  template <CxxUtils::detail::RefCounted OTHER>
  requires std::convertible_to<OTHER*, T*>
  RefCountedPtr (RefCountedPtr<OTHER>&& other);

  /// Assignment.
  RefCountedPtr& operator= (const RefCountedPtr& other);

  /// Move.
  RefCountedPtr& operator= (RefCountedPtr&& other);

  /// Assignment from pointer.
  RefCountedPtr& operator= (T* p);

  /// Assignment from different type.
  template <CxxUtils::detail::RefCounted OTHER>
  requires std::convertible_to<OTHER*, T*>
  RefCountedPtr& operator= (const RefCountedPtr<OTHER>& other);

  /// Move from different type.
  template <CxxUtils::detail::RefCounted OTHER>
  requires std::convertible_to<OTHER*, T*>
  RefCountedPtr& operator= (RefCountedPtr<OTHER>&& other);

  /// Destructor.
  ~RefCountedPtr();

  /// Change the pointer.
  void reset (T* ptr = nullptr);

  /// Check if the pointer is valid.
  bool isValid() const;

  /// Check if the pointer is valid.
  explicit operator bool() const;

  /// Check if the pointer is invalid.
  bool operator!() const;

  /// Get the pointer.
  T* get();
  const T* get() const;

  /// Convert to pointer.
  operator T*();
  operator const T*() const;

  /// Dereference.
  T* operator->();
  const T* operator->() const;

  /// Dereference.
  T& operator*();
  const T& operator*() const;


private:
  template <CxxUtils::detail::RefCounted OTHER> friend class RefCountedPtr;

  // Return the pointer and give up ownership.
  T* release();

  /// Pointer.
  T* m_ptr = nullptr;
};


} // namespace CxxUtils


#include "CxxUtils/RefCountedPtr.icc"


#endif // not CXXUTILS_REFCOUNTEDPTR_H
