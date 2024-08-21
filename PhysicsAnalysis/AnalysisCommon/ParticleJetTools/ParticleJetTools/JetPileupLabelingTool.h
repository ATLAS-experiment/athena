/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETTOOLS_JETPILEUPLABELINGTOOL_H
#define PARTICLEJETTOOLS_JETPILEUPLABELINGTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "JetInterface/IJetDecorator.h"


class JetPileupLabelingTool :
  public asg::AsgTool,
  virtual public IJetDecorator
{
  ASG_TOOL_CLASS(JetPileupLabelingTool, IJetDecorator)

public:
  
  // Apparently this defines a constructor
  using asg::AsgTool::AsgTool;

  virtual StatusCode initialize() override;

  /// Decorate hard-scatter and pileup labels to a jet collection
  StatusCode decorate(const xAOD::JetContainer& jets) const override;

  /// Print configured parameters
  void print() const override;

private:

  Gaudi::Property<std::string> m_jetContainerName{this, "RecoJetContainer", "AntiKt4EMPFlowJets", "Input reco jet container name"};

  Gaudi::Property<bool> m_suppressOutputDeps{this, "SuppressOutputDependence", false, "Ignore creating the output decoration dependency for data flow; for analysis"};

  Gaudi::Property<float> m_hsMaxDR{this, "isHSMaxDR", 0.3, "Tag a reco jet as HS if it is at most this distance from a truth jet"};
  Gaudi::Property<float> m_hsMinPt{this, "isHSMinPt", 10e3, "Only consider truth jets above this pT for HS tagging"};
  Gaudi::Property<float> m_puMinDR{this, "isPUMinDR", 0.6, "Tag a reco jet as PU if it is at least this distance from any truth jet"};
  Gaudi::Property<float> m_puMinPt{this, "isPUMinPt", 0, "Only consider truth jets above this pT to veto jets for being PU tagged"};

  SG::ReadHandleKey<xAOD::JetContainer> m_truthJetsKey{this, "TruthJetContainer", "AntiKt4TruthDressedWZJets", "SG key for input Truth HS jets"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_decIsHSKey{this, "isHSLabel", "isJvtHS", "SG key for label of HS jets"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_decIsPUKey{this, "isPULabel", "isJvtPU", "SG key for label of PU jets"};

};

#endif
