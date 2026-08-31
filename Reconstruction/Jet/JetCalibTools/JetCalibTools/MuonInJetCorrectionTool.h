///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MuonInJetCorrectionTool.h 
// Header file for class MuonInJetCorrectionTool
// Authors: Mohamed Belfkir <mohamed.belfkir@cern.ch>
//          Thomas Strebler <thomas.strebler@cern.ch>
///////////////////////////////////////////////////////////////////

#ifndef JETCALIBTOOLS_APPLYMUONINJETCORRECTION_H
#define JETCALIBTOOLS_APPLYMUONINJETCORRECTION_H 1

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "JetAnalysisInterfaces/IMuonInJetCorrectionTool.h"

#include "xAODJet/Jet.h"
#include "xAODMuon/Muon.h"

#include <vector>


class MuonInJetCorrectionTool : public asg::AsgTool,
				virtual public IMuonInJetCorrectionTool {

  ASG_TOOL_CLASS1(MuonInJetCorrectionTool, IMuonInJetCorrectionTool)

public:
  /// Constructor with parameters: 
  MuonInJetCorrectionTool(const std::string& name);

  virtual StatusCode applyMuonInJetCorrection(xAOD::Jet& jet, const std::vector<const xAOD::Muon*>& muons, int& nmuons) const override;

private:

  const xAOD::Muon* getMuonInJet(const xAOD::Jet& jet, const std::vector<const xAOD::Muon*>& muons, int& nmuons) const;

  Gaudi::Property<bool> m_changeAngularComponents{this, "changeAngularComponents", false, "Change angular components of the jet 4-vector"};
  Gaudi::Property<double> m_Jet_Muon_dR{this, "Jet_Muon_dR", 0.4, "Muon-matching dR cut"};
  Gaudi::Property<bool> m_doVR{this, "doVR", true, "Use variable radius for muon matching"};
  Gaudi::Property<bool> m_doLargeR{this, "doLargeR", false, "Do correction for large-R jets"};

};
#endif //> !JETCALIBTOOLS_APPLYMUONINJETCORRECTION_H

