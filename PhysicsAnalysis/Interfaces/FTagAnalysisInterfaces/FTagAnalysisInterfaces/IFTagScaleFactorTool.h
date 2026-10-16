/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FTAG_CPIFTAGSCALEFACTORTOOL_H
#define FTAG_CPIFTAGSCALEFACTORTOOL_H

#include "PATInterfaces/IReentrantSystematicsTool.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/CorrectionCode.h"
#include "xAODBase/IParticle.h"
#include <map>
#include <vector>

namespace ftag{
/// @brief Interface for the FTag scale factor tool.
/// 
/// Takes in an xAOD object and returns its scale factor
/// for a given systematic variation.

class IFTagScaleFactorTool : virtual public CP::IReentrantSystematicsTool {
  ASG_TOOL_INTERFACE ( ftag::IFTagScaleFactorTool )

  public:
  virtual CP::CorrectionCode getScaleFactor(const xAOD::IParticle& p, float& sf, const CP::SystematicSet& sys) const = 0;

  /// @brief Infer the working-point and Continuous quantile of a jet. 
  ///
  /// Reutns a map from WP name to bin index: 0 or 1 for fixed-cut 
  /// working points, and an integr bin quantile for pseudo-continous.
  virtual std::unordered_map<std::string, int> inferWPs( const xAOD::IParticle* p ) const = 0; 

}; // class IFTagScaleFactorTool
}
#endif // FTAG_CPIFTAGSCALEFACTORTOOL_H
