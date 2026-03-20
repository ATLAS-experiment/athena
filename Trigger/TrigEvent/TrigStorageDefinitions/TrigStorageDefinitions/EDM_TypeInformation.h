/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRIGTYPEINFO_EDM_TYPEINFORMATION_H
#define TRIGTYPEINFO_EDM_TYPEINFORMATION_H

#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include "TrigStorageDefinitions/TypeInformation.h"

#define HLT_BEGIN_TYPE_REGISTRATION typedef HLT::TypeInformation::List<>

/// Helper to handle variable number of arguments
#define _HLT_REGISTER_INTERNAL(OBJECT, FEATURE, CONTAINER, ...) \
  ::merge<HLT::TypeInformation::TypeInfo<OBJECT, HLT::TypeInformation::List<FEATURE>, CONTAINER __VA_OPT__(, ) __VA_ARGS__>>

/**
 * Register an HLT type with optional Aux container.
 *
 * HLT_REGISTER_TYPE(OBJECT, FEATURE, CONTAINER [, AUX])
 */
#define HLT_REGISTER_TYPE(OBJECT, FEATURE, CONTAINER, ...) \
  _HLT_REGISTER_INTERNAL(OBJECT, FEATURE, CONTAINER __VA_OPT__(, ) __VA_ARGS__)

#define HLT_END_TYPE_REGISTRATION(name) TypeInfo_##name;


/**
 * Register a type map for a package.
 */
#define TYPEMAPCLASS(name) \
  struct class_##name { using map = TypeInfo_##name; };

#endif
