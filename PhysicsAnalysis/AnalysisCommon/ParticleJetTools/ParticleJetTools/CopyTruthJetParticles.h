/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COPYTRUTHJETPARTICLES_H
#define COPYTRUTHJETPARTICLES_H

#include "AsgDataHandles/ReadDecorHandleKeyArray.h"
#include "ParticleJetTools/CopyTruthParticles.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "xAODTruth/TruthParticle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include <vector>
#include <map>

class CopyTruthJetParticles : public CopyTruthParticles {
ASG_TOOL_CLASS2(CopyTruthJetParticles, CopyTruthParticles, IJetExecuteTool)
public:

  /// Constructor
  CopyTruthJetParticles(const std::string& name);

  /// Function initialising the tool
  virtual StatusCode initialize();


  /// redefine execute so we can call our own classify() with the barcode offset for the current event.
  virtual int execute() const;

  /// Redefine our own Classifier function(s)
  bool classifyJetInput(const xAOD::TruthParticle* tp, 
                        std::vector<const xAOD::TruthParticle*>& promptLeptons,
                        std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const;

  // metadata check
  int setBarCodeFromMetaDataCheck() const;

  /// The base classify() is not used 
  bool classify(const xAOD::TruthParticle* ) const {return false;}
  
private:
 
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

  unsigned int getTCresult(const xAOD::TruthParticle* tp,
                           std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const;
 
  /// Maximum allowed eta for particles in jets
  Gaudi::Property<float> m_maxAbsEta{this, "MaxAbsEta" , 5.};

  Gaudi::Property<std::vector<int>> m_vetoPDG_IDs{this, "VetoPDG_IDs", {},
              "List of PDG IDs (python list) to veto.  Will ignore these and all children of these."};
  bool comesFrom( const xAOD::TruthParticle* tp, const int pdgID, std::vector<int>& used_vertices ) const;

  /// Name of the decoration to be used for identifying FSR (dressing) photons
  SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer> m_dressingNames{this, "DressingDecorationNames", {}, 
  "Name of the dressed photon decoration (if one should be used)"};

  /// Handle on MCTruthClassifier for finding prompt leptons
  ToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", ""};
};


#endif
