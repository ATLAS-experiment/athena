// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_XAODTYPES_H
#define ATHDEVICEUTILS_XAODTYPES_H

// Project include(s).
#include "AthContainers/AuxElement.h"

namespace AthDeviceUtilsTest {

/// xAOD type for a container without any jagged vectors
class SimplexAODObject : public SG::AuxElement {

 public:
  // Inherit the base class's constructor(s).
  using SG::AuxElement::AuxElement;

  /// @name Accessor function(s)
  /// @{

  int intVar() const;
  void setIntVar(int value);

  float floatVar() const;
  void setFloatVar(float value);

  /// @}

};  // class SimplexAODObject

class JaggedxAODObject : public SimplexAODObject {

 public:
  // Inherit the base class's constructor(s).
  using SimplexAODObject::SimplexAODObject;

  /// @name Accessor function(s)
  /// @{

  const std::vector<unsigned int>& unsignedVectorVar() const;
  void setUnsignedVectorVar(const std::vector<unsigned int>& value);

  const std::vector<double>& doubleVectorVar() const;
  void setDoubleVectorVar(const std::vector<double>& value);

  /// @}

};  // class JaggedxAODObject

}  // namespace AthDeviceUtilsTest

#endif  // ATHDEVICEUTILS_XAODTYPES_H
