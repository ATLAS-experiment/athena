/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETTOOLS_FTAGLARGERJETTRUTHLABELTOOL_H
#define PARTICLEJETTOOLS_FTAGLARGERJETTRUTHLABELTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "JetInterface/IJetDecorator.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleFwd.h"
#include "ParticleJetTools/FtagLargeRJetLabelEnum.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"


#include <set>
#include <string>
#include <vector>


/// Simplified FTAG large-R jet truth labelling tool.
///
/// Labels jets with a factorized Origin x Decay x Containment scheme
/// using ghost associations only (no kinematic cuts). Origin is the
/// ghost-matched parent; Decay is from its truth decay chain;
/// Containment is checked via ghost hadron counting on the jet.

class FtagLargeRJetTruthLabelTool : public asg::AsgTool,
                                     virtual public IJetDecorator {
  ASG_TOOL_CLASS(FtagLargeRJetTruthLabelTool, IJetDecorator)

public:
  FtagLargeRJetTruthLabelTool(const std::string& name);
  virtual StatusCode initialize() override;
  virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

private:
  FtagLargeRLabel::TypeEnum classifyDecay(const xAOD::TruthParticle* parent, int originPdgId, int nB, int nC, int nTau, int extLabel) const;
  FtagLargeRLabel::TypeEnum classifyHiggsDecay(const std::set<int>& ids, const std::vector<const xAOD::TruthParticle*>& children, int nB, int nC, int nTau, int extLabel) const;
  FtagLargeRLabel::TypeEnum classifyTopDecay(const std::vector<const xAOD::TruthParticle*>& children, int nB, int nC) const;
  FtagLargeRLabel::TypeEnum classifyWDecay(const std::vector<const xAOD::TruthParticle*>& children, int nC) const;
  FtagLargeRLabel::TypeEnum classifyZDecay(const std::set<int>& ids, int nB, int nC, int nTau, int extLabel) const;
  FtagLargeRLabel::TypeEnum classifyQCD(int nB, int nC) const;

  std::vector<const xAOD::TruthParticle*> getDecayProducts(const xAOD::TruthParticle* parent) const;
  std::vector<const xAOD::TruthParticle*> getDecayProducts(
      const xAOD::TruthParticle* parent,
      std::set<const xAOD::TruthParticle*>& visited) const;

  SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{this, "JetContainer", "", "Input jet container"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_decLabel{this, "DecLabel", m_jetsKey, "FtagLargeRTruthLabel", "Decoration name for the truth label"};
};

#endif
