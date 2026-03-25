/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IRCSAFELABEL_H
#define IRCSAFELABEL_H

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetDecorator.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "fastjet/JetDefinition.hh"
#include "fastjet/Selector.hh"
#include "fastjet/contrib/FlavInfo.hh"
#include "ParticleJetTools/ParticleJetLabelCommon.h"
#include "fastjet/PseudoJet.hh"

#include <memory>

class IRCSafeLabelTool : public asg::AsgTool, public IJetDecorator {
ASG_TOOL_CLASS(IRCSafeLabelTool, IJetDecorator)
public:

  /// Constructor
  IRCSafeLabelTool(const std::string& name);

  StatusCode initialize() override;

  StatusCode decorate(const xAOD::JetContainer& jets) const override;



protected:
  std::vector< std::vector<fastjet::PseudoJet> > getJetInputs(
                                                   const xAOD::TruthParticleContainer& parts,
                                                   const xAOD::TruthParticleContainer& label_bs,
                                                   const xAOD::TruthParticleContainer& label_cs) const;

  /// Function matching truth pseudojets to reco jets
  std::vector< std::vector<const fastjet::PseudoJet*> > match(
                                                    std::vector<fastjet::PseudoJet>& tagged_jets,
                                                    const xAOD::JetContainer& jets) const;

  /// Name of jet label attributes
  ParticleJetTools::IRCSafeLabelNames m_ircsafelabelnames;
  std::unique_ptr<ParticleJetTools::IRCSafeLabelDecorators> m_ircsafelabeldecs;

  /// Read handles particle collections for labeling. We need all inputs used for jet clustering
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_bottomPartCollectionKey{this,"BParticleCollection","","ReadHandleKey for bottomPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_charmPartCollectionKey{this,"CParticleCollection","","ReadHandleKey for charmPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_outTruthPartKey{this, "TruthParticleCollection","", "ReadHandleKey of the TruthParticle collection"};

  /// Minimum pT for truth jet to be matched (in MeV)
  double m_truthjetptmin;

  /// Minimum pT for jet selection (in MeV)
  double m_jetptmin;

  /// Maximum dR for matching criterion
  double m_drmax;

  /// Radius of truth jets with undecayed B and D hadrons
  double m_truthR;

  /// Enabled algorithms (subset of: IFN, CMP, GHS, SDF, AKT)
  std::vector<std::string> m_enabledAlgos;

  bool m_doIFN{true};
  bool m_doCMP{true};
  bool m_doGHS{true};
  bool m_doSDF{true};
  bool m_doAKT{true};

  // FastJet configuration cached at initialize to avoid per-event allocations.
  std::unique_ptr<fastjet::Selector> m_selectPt;
  std::unique_ptr<fastjet::contrib::FlavRecombiner> m_flavRecombiner;
  std::unique_ptr<fastjet::JetDefinition> m_aktJetDef;
  std::unique_ptr<fastjet::JetDefinition> m_ifnJetDef;
  std::unique_ptr<fastjet::JetDefinition> m_cmpJetDef;

};


#endif
