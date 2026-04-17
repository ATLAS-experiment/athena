/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_IDITAUSMEARINGTOOL_H
#define TAUANALYSISTOOLS_IDITAUSMEARINGTOOL_H

/*
  author: David Kirchmeier
  mail: david.kirchmeier@cern.ch
  ported in R22 by: Antonio De Maria
  mail: antonio.de.maria@cern.ch
  documentation in: ../doc/README-DiTauSmearingTool.rst
*/

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"

namespace TauAnalysisTools
{

class IDiTauSmearingTool
  : public virtual asg::IAsgTool	
  , public virtual CP::ISystematicsTool
{

  /// Declare the interface that the class provides
  ASG_TOOL_INTERFACE( TauAnalysisTools::IDiTauSmearingTool )

public:

  /// Apply the correction on a modifiable object
  virtual CP::CorrectionCode applyCorrection( xAOD::DiTauJet& xDiTau ) const = 0;
  /// Create a corrected copy from a constant ditau
  virtual CP::CorrectionCode correctedCopy( const xAOD::DiTauJet& input,
      xAOD::DiTauJet*& output ) const = 0;

}; // class IDiTauSmearingTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_IDITAUSMEARINGTOOL_H

