/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PMGTOOLS_ISYSTRUTHWEIGHTTOOLTOOL_H
#define PMGTOOLS_ISYSTRUTHWEIGHTTOOLTOOL_H

// STL include(s):
#include <string>
#include <vector>

// EDM include(s):
#include "PATInterfaces/ISystematicsTool.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace PMGTools
{
  /// Interface for xAOD Truth Weight Tool which returns
  /// an event weight based on truth particle container
  ///
  /// @author Miha Muskinja
  ///
  class ISysTruthWeightTool : public virtual CP::IReentrantSystematicsTool
  {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(xAOD::ISysTruthWeightTool)

  public:
    /// Return the weight correponding to the current systematic variation
    virtual float getSysWeight(const xAOD::TruthParticleContainer *truthParticles, const CP::SystematicSet &sys) const = 0;
  };  // class ISysTruthWeightTool

}  // namespace PMGTools

#endif  // PMGTOOLS_ISYSTRUTHWEIGHTTOOLTOOL_H
