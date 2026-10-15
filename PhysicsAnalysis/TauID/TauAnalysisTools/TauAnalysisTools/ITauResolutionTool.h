/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_ITAURESOLUTIONTOOL_H
#define TAUANALYSISTOOLS_ITAURESOLUTIONTOOL_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODTau/TauJet.h"

namespace TauAnalysisTools
{

class ITauResolutionTool
  : public virtual asg::IAsgTool
{

  /// Declare the interface that the class provides
  ASG_TOOL_INTERFACE( TauAnalysisTools::ITauResolutionTool )


}; // class ITauRsolutionTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_ITAURESOLUTIONTOOL_H
