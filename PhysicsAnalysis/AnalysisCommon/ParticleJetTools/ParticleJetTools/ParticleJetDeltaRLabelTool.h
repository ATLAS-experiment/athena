/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETDELTARLABELTOOL_H
#define PARTICLEJETDELTARLABELTOOL_H

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetDecorator.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

#include "ParticleJetTools/ParticleJetLabelCommon.h"

class ParticleJetDeltaRLabelTool : public asg::AsgTool, public IJetDecorator {
ASG_TOOL_CLASS(ParticleJetDeltaRLabelTool, IJetDecorator)
public:

  /// Constructor
  ParticleJetDeltaRLabelTool(const std::string& name);

  StatusCode initialize() override;

  StatusCode decorate(const xAOD::JetContainer& jets) const override;

  std::vector<std::vector<const xAOD::TruthParticle*> > match(
          const xAOD::TruthParticleContainer& parts,
          const xAOD::JetContainer& jets) const;


protected:

  /// Name of jet label attributes
  ParticleJetTools::LabelNames m_labelnames;
  std::unique_ptr<ParticleJetTools::LabelDecorators> m_labeldecs;
  std::string m_taulabelname;
  std::string m_bottomlabelname;
  std::string m_charmlabelname;

  /// Read handles particle collections for labeling
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_tauPartCollectionKey{this,"TauParticleCollection","","ReadHandleKey for tauPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_bottomPartCollectionKey{this,"BParticleCollection","","ReadHandleKey for bottomPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_charmPartCollectionKey{this,"CParticleCollection","","ReadHandleKey for charmPartCollection"};
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventsKey{
    this, "TruthEventCollection", "TruthEvents",
    "Truth event collection"
  };

  // linkers to the truth particles
  std::unique_ptr<ParticleJetTools::IParticleLinker> m_blinker;
  std::unique_ptr<ParticleJetTools::IParticleLinker> m_clinker;
  std::unique_ptr<ParticleJetTools::IParticleLinker> m_taulinker;

  /// Minimum pT for particle selection (in MeV)
  double m_partptmin;

  /// Minimum pT for jet selection (in MeV)
  double m_jetptmin;

  /// Maximum dR for matching criterion
  double m_drmax;

  /// Matching mode: can be MinDR or MaxPt
  std::string m_matchmode;
};


#endif
