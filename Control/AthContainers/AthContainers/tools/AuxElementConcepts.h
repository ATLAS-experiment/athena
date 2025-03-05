// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/AuxElementConcepts.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Concepts for classes compatible with AuxElement / ConstAuxElement.
 *
 * The Accessor/Decorator classes were originally defined as members
 * of AuxElement.  For backwards compatibility, we still make those names
 * available there via aliases.  But if the Accessor classes depend on
 * AuxElement, then that creates a circularity.  To break this,
 * we template the Accessor methods that would take [Const]AuxElement
 * and add concept requirements that the template argument is compatible
 * with the AuxElement interface.
 */


#ifndef ATHCONTAINERS_AUXELEMENTCONCEPTS_H
#define ATHCONTAINERS_AUXELEMENTCONCEPTS_H


#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/AuxVectorData.h"


namespace SG {


/// Test if T is compatible with ConstAuxElement.
template <class T>
concept IsConstAuxElement =
  requires (const T& elt)
  {
    { elt } -> std::convertible_to<const IAuxElement&>;
    { elt.container() } -> std::convertible_to<const AuxVectorData*>;
  };


/// Test if T is compatible with AuxElement.
template <class T>
concept IsAuxElement =
  requires (T& elt)
  {
    { elt } -> std::convertible_to<IAuxElement&>;
    { elt.container() } -> std::convertible_to<AuxVectorData*>;
  };


} // namespace SG


#endif // not ATHCONTAINERS_AUXELEMENTCONCEPTS_H
