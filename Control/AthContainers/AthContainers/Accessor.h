// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/Accessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Helper class to provide type-safe access to aux data.
 *
 * To avoid circularities, this file must not include AuxElement.h.
 * Methods which would normally take AuxElement arguments
 * are instead templated.
 */


#ifndef ATHCONTAINERS_ACCESSOR_H
#define ATHCONTAINERS_ACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include <string>
#include <typeinfo>


namespace SG {


/**
 * @brief Helper class to provide type-safe access to aux data.
 *
 * This is written as a separate class in order to be able
 * to cache the name -> auxid lookup.
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   SG::Accessor<int> vint1 ("myInt");
 *   ...
 *   const Myclass* m = ...;
 *   int x = vint1 (*m);
 @endcode
 *
 * You can also use this to define getters/setters in your class:
 *
 *@code
 *  class Myclass {
 *    ...
 *    int get_x() const
 *    { const static Accessor<int> acc ("x", "Myclass");
 *      return acc (*this); }
 *    int& get_x()
 *    { const static Accessor<int> acc ("x", "Myclass");
 *      return acc (*this); }
 @endcode
*/
template <class T, class ALLOC = AuxAllocator_t<T> >
class Accessor
  : public ConstAccessor<T, ALLOC>
{
public:
  /// Type referencing an item.
  using reference_type = typename AuxDataTraits<T, ALLOC>::reference_type;

  /// Type the user sees.
  using element_type = typename AuxDataTraits<T, ALLOC>::element_type;

  /// Pointer into the container holding this item.
  using container_pointer_type =
    typename AuxDataTraits<T, ALLOC>::container_pointer_type;

  /// A span over elements in the container.
  using span = typename AuxDataTraits<T, ALLOC>::span;

  using ConstAccessor<T, ALLOC>::operator();
  using ConstAccessor<T, ALLOC>::getDataArray;
  using ConstAccessor<T, ALLOC>::getDataSpan;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param e The element for which to fetch the variable.
   */
  template <class ELT>
  ATH_REQUIRES( IsAuxElement<ELT> )
  reference_type operator() (ELT& e) const;


  /**
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   * Looping over the index via this method will be faster then
   * looping over the elements of the container.
   */
  reference_type operator() (AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to fetch the variable.
   * @param x The variable value to set.
   */
  template <class ELT>
  ATH_REQUIRES( IsAuxElement<ELT> )
  void set (ELT& e, const element_type& x) const;


  /**
   * @brief Get a pointer to the start of the auxiliary data array.
   * @param container The container from which to fetch the variable.
   */
  container_pointer_type getDataArray (AuxVectorData& container) const;


  /**
   * @brief Get a span over the auxilary data array.
   * @param container The container from which to fetch the variable.
   */
  span
  getDataSpan (AuxVectorData& container) const;
    

  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param e An element of the container which to test the variable.
   */
  template <class ELT>
  ATH_REQUIRES( IsAuxElement<ELT> )
  bool isAvailableWritable (ELT& e) const;
};


} // namespace SG


#include "AthContainers/Accessor.icc"


#endif // not ATHCONTAINERS_ACCESSOR_H
