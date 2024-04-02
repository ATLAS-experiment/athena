// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/TypelessConstAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2023
 * @brief Helper class to provide const generic access to aux data.
 *
 * To avoid circularities, this file must not include AuxElement.h.
 * Methods which would normally take ConstAuxElement arguments
 * are instead templated.
 */


#ifndef ATHCONTAINERS_TYPELESSCONSTACCESSOR_H
#define ATHCONTAINERS_TYPELESSCONSTACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include <string>
#include <typeinfo>


namespace SG {


/**
 * @brief Helper class to provide const generic access to aux data.
 *
 * This is written as a separate class in order to be able
 * to cache the name -> auxid lookup.
 *
 * This should generally only be used by code which treats
 * auxiliary data generically (that is, where the type is not
 * known at compile-time).  Most of the time, you'd want to use
 * the type-safe versions @c ConstAccessor and @c Accessor.
 */
class TypelessConstAccessor
{
public:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  TypelessConstAccessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param ti The type for this aux data item.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  TypelessConstAccessor (const std::type_info& ti,
                         const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  TypelessConstAccessor (const std::string& name,
                         const std::string& clsname);


  /**
   * @brief Constructor.
   * @param ti The type for this aux data item.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  TypelessConstAccessor (const std::type_info& ti,
                         const std::string& name,
                         const std::string& clsname);


  /**
   * @brief Fetch the variable for one element, as a const pointer.
   * @param e The element for which to fetch the variable.
   */
  template <class ELT>
  ATH_REQUIRES( IsConstAuxElement<ELT> )
  const void* operator() (const ELT& e) const;
    

  /**
   * @brief Fetch the variable for one element, as a const pointer.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   * Looping over the index via this method will be faster then
   * looping over the elements of the container.
   */
  const void* operator() (const AuxVectorData& container, size_t index) const;

    
  /**
   * @brief Get a pointer to the start of the auxiliary data array.
   * @param container The container from which to fetch the variable.
   */
  const void* getDataArray (const AuxVectorData& container) const;
    

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
  /// The cached @c auxid.
  SG::auxid_t m_auxid;

  /// Cached element size.
  size_t m_eltSize;
};


} // namespace SG


#include "AthContainers/TypelessConstAccessor.icc"


#endif // not ATHCONTAINERS_TYPELESSCONSTACCESSOR_H
