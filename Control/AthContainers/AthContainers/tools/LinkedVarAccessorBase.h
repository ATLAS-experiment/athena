// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/LinkedVarAccessorBase.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Base class usable for accessors for variables with linked variables.
 */


#ifndef ATHCONTAINERS_LINKEDVARACCESSORBASE_H
#define ATHCONTAINERS_LINKEDVARACCESSORBASE_H


#include "AthContainers/tools/AuxElementConcepts.h"
#include "AthContainersInterfaces/AuxTypes.h"
#include <cstdint>


namespace SG { namespace detail {


/**
 * @brief Base class usable for accessors for variables with linked variables.
 */
class LinkedVarAccessorBase
{
public:
  /**
   * @brief Test to see if this variable exists in the store.
   * @param e An element of the container which to test the variable.
   */
  template <class ELT>
  requires (IsConstAuxElement<ELT>)
  bool isAvailable (const ELT& e) const;


  /**
   * @brief Return the aux id for this variable.
   */
  SG::auxid_t auxid() const;


  /**
   * @brief Return the aux id for the linked payload vector.
   */
  SG::auxid_t linkedAuxid() const;


protected:
  // Store these as uint32_t rather than auxid_t since we're going to want
  // to be able to create temporary instances of this.
  /// The cached @c auxid for the linked payload vector.
  uint32_t m_linkedAuxid = 0;

  /// The cached @c auxid.
  uint32_t m_auxid = 0;
};


} } // namespace SG::detail


#include "AthContainers/tools/LinkedVarAccessorBase.icc"


#endif // not ATHCONTAINERS_LINKEDVARACCESSORBASE_H
