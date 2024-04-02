// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/ConstAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Helper class to provide constant type-safe access to aux data.
 *
 * To avoid circularities, this file must not include AuxElement.h.
 * Methods which would normally take ConstAuxElement arguments
 * are instead templated.
 */


#ifndef ATHCONTAINERS_CONSTACCESSOR_H
#define ATHCONTAINERS_CONSTACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include <string>
#include <typeinfo>


namespace SG {


/**
 * @brief Helper class to provide constant type-safe access to aux data.
 *
 * This is written as a separate class in order to be able
 * to cache the name -> auxid lookup.
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   SG::ConstAccessor<int> vint1 ("myInt");
 *   ...
 *   const Myclass* m = ...;
 *   int x = vint1 (*m);
 @endcode
 *
 * This class can be used only for reading data.
 * To modify data, see the class @c Accessor.
 */
template <class T, class ALLOC = AuxAllocator_t<T> >
class ConstAccessor
{
public:
  /// Type the user sees.
  using element_type = typename AuxDataTraits<T, ALLOC>::element_type;

  /// Type referencing an item.
  using const_reference_type =
    typename AuxDataTraits<T, ALLOC>::const_reference_type;

  /// Pointer into the container holding this item.
  using const_container_pointer_type =
    typename AuxDataTraits<T, ALLOC>::const_container_pointer_type;

  /// A span over elements in the container.
  using const_span = typename AuxDataTraits<T, ALLOC>::const_span;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Fetch the variable for one element, as a const reference.
   * @param e The element for which to fetch the variable.
   */
  template <class ELT>
  ATH_REQUIRES( IsConstAuxElement<ELT> )
  const_reference_type operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element, as a const reference.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   * Looping over the index via this method will be faster then
   * looping over the elements of the container.
   */
  const_reference_type
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a pointer to the start of the auxiliary data array.
   * @param container The container from which to fetch the variable.
   */
  const_container_pointer_type
  getDataArray (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the auxilary data array.
   * @param container The container from which to fetch the variable.
   */
  const_span
  getDataSpan (const AuxVectorData& container) const;
    

  /**
   * @brief Test to see if this variable exists in the store.
   * @param e An element of the container which to test the variable.
   */
  template <class ELT>
  ATH_REQUIRES( IsConstAuxElement<ELT> )
  bool isAvailable (const ELT& e) const;


  /**
   * @brief Return the aux id for this variable.
   */
  SG::auxid_t auxid() const;


protected:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See AuxTypeRegsitry.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name,
                 const std::string& clsname,
                 const SG::AuxTypeRegistry::Flags flags);


  /// The cached @c auxid.
  SG::auxid_t m_auxid;
};


} // namespace SG


#include "AthContainers/ConstAccessor.icc"


#endif // not ATHCONTAINERS_CONSTACCESSOR_H
