/**
 * 
 * @copyright Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 * 
 * @file ITauHFVetoTool.h
 * @author Peng Wang (peng.wang@cern.ch)
 * @brief 
 * @date 2024-11-07
 */

#ifndef TAUANALYSISTOOLS_ITAUHFVETOTOOL_H
#define TAUANALYSISTOOLS_ITAUHFVETOTOOL_H

// framework include(s)
#include "AsgTools/IAsgTool.h"

// EDM include(s)
#include "xAODCore/AuxContainerBase.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODBTagging/BTaggingUtilities.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

namespace TauAnalysisTools
{

class ITauHFVetoTool : public virtual asg::IAsgTool
{
  ASG_TOOL_INTERFACE( TauAnalysisTools::ITauHFVetoTool )

public:

  virtual StatusCode applyHFvetoBDTs(const xAOD::TauJetContainer* Taus, const xAOD::JetContainer* PFlowJets) const = 0;
  virtual const xAOD::Jet* findClosestPFlowJet(const xAOD::TauJet* xTau, const xAOD::JetContainer* vPFlowJets) const = 0;
  virtual std::vector<float> assembleInputValues(const xAOD::TauJet* xTau, const xAOD::Jet* xAuxJet) const = 0;

};

}

#endif 