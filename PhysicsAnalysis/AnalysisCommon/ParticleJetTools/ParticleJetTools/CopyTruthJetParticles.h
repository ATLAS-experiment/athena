/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COPYTRUTHJETPARTICLES_H
#define COPYTRUTHJETPARTICLES_H

#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetExecuteTool.h"
#include "AsgDataHandles/ReadDecorHandleKeyArray.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "xAODTruth/TruthParticle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include <vector>
#include <map>

class CopyTruthJetParticles : public IJetExecuteTool, public asg::AsgTool {
  ASG_TOOL_INTERFACE(CopyTruthJetParticles)
  ASG_TOOL_CLASS(CopyTruthJetParticles, IJetExecuteTool)
  public:

  /// Constructor
  CopyTruthJetParticles(const std::string& name);

  /// Function initialising the tool
  virtual StatusCode initialize() override final;

  /// redefine execute so we can call our own classify()
  virtual int execute() const override final;

private:
  /// Redefine our own Classifier function(s)
  bool classifyJetInput(const xAOD::TruthParticle* tp,
                        std::vector<const xAOD::TruthParticle*>& promptLeptons,
                        std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const;

  unsigned int getTCresult(const xAOD::TruthParticle* tp,
                           std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const;

  bool comesFrom( const xAOD::TruthParticle* tp, const int pdgID, std::vector<int>& used_vertices ) const;

  /// Handle on MCTruthClassifier for finding prompt leptons
  ToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", ""};

  /// Key for input truth event
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey{this, "TruthParticleKey", "TruthParticles", "SG Key for input truth particle container"};

  /// Name of the decoration to be used for identifying FSR (dressing) photons
  SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer> m_dressingNames{this, "DressingDecorationNames", {},  "Name of the dressed photon decoration (if one should be used)"};

  /// Key for output truth particles
  SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer> > m_outTruthPartKey{this, "OutputName", "TagInputs", "Name of the resulting TruthParticle collection"};

  /// Minimum pT for particle selection (in MeV)
  Gaudi::Property<float> m_ptmin{this, "PtMin", 0. , "Minimum pT of particles to be accepted for tagging (in MeV)"};

  /// Maximum allowed eta for particles in jets
  Gaudi::Property<float> m_maxAbsEta{this, "MaxAbsEta" , 5.};

  Gaudi::Property<std::vector<int>> m_vetoPDG_IDs{this, "VetoPDG_IDs", {},
    "List of PDG IDs (python list) to veto.  Will ignore these and all children of these."};

  // Options for storate
  Gaudi::Property<bool> m_includeBSMNonInt{this, "IncludeBSMNonInteracting", false,
    "Include noninteracting BSM particles (excluding neutrinos) in the output collection"};
  Gaudi::Property<bool> m_includeNu{this,"IncludeNeutrinos", false,
    "Include neutrinos in the output collection"};
  Gaudi::Property<bool> m_includeMu{this, "IncludeMuons", false,
    "Include muons in the output collection"};
  Gaudi::Property<bool> m_includePromptLeptons{this, "IncludePromptLeptons", true,
    "Include leptons from prompt decays (i.e. not from hadron decays) in the output collection"};
  Gaudi::Property<bool> m_includePromptPhotons{this, "IncludePromptPhotons", true,
    "Include photons from Higgs and other decays that produce isolated photons"};
  Gaudi::Property<bool> m_chargedOnly{this, "ChargedParticlesOnly", false,
    "Include only charged particles in the output collection" };
  // -- added for dark jet clustering -- //
  Gaudi::Property<bool> m_includeSM{this, "IncludeSMParts", true,
    "Include SM particles in the output collection"};
  Gaudi::Property<bool> m_includeDark{this, "IncludeDarkHads", false,
    "Include dark hadrons in the output collection"};

  // ATLASRECTS-8290: this is for backward compatability, remove eventually
  Gaudi::Property<bool> m_use_barcode {
    this, "useBarcode", false, "use barcode rather than UID"
  };
  SG::ConstAccessor<int> m_uid{"uid"};

};


#endif
