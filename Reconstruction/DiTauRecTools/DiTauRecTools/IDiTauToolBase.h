/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef DITAURECTOOLS_IDITAUTOOLBASE_H
#define DITAURECTOOLS_IDITAUTOOLBASE_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s)
#include "xAODTau/DiTauJet.h"
#include <string>

namespace DiTauRecTools
{

  class IDiTauToolBase :
    public virtual asg::IAsgTool
  {

    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE( tauRecTools::IDiTauToolBase )

    public:
    // calculate ID variables
    virtual StatusCode execute(const xAOD::DiTauJet& xDiTau) const = 0;
  }; // class IDiTauToolBase

} // namespace DiTauRecTools

#endif // DITAURECTOOLS_IDITAUTOOLBASE_H


