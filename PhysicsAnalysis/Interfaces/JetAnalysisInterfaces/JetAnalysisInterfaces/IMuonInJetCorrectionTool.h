/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IMuonInJetCorrectionTool.h

//////////////////////////////////////////////////////////
/// class IMuonInJetCorrectionTool
/// 
/// Interface for the tool that applies muon-in-jet corrections.
///
//////////////////////////////////////////////////////////

#ifndef JETCALIBTOOL_IMUONINJETCORRECTIONTOOL_H
#define JETCALIBTOOL_IMUONINJETCORRECTIONTOOL_H

#include "AsgTools/IAsgTool.h"

#include "xAODJet/Jet.h"
#include <vector>
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"

class IMuonInJetCorrectionTool : public virtual asg::IAsgTool {
  ASG_TOOL_INTERFACE(CP::IMuonInJetCorrectionTool)

public:

  virtual StatusCode applyMuonInJetCorrection(xAOD::Jet& jet, const std::vector<const xAOD::Muon*>& muons, int& nmuons) const = 0;

}; 

#endif //> !JETCALIBTOOL_IMUONINJETCORRECTIONTOOL_H
