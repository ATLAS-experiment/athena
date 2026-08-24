/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETIRCSafeLabelTool_H
#define JETIRCSafeLabelTool_H

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
#include "AsgTools/PropertyWrapper.h"

#include <memory>
#include <array>
#include <string>
#include <vector>

class JetIRCSafeLabelTool : public asg::AsgTool, public IJetDecorator {
ASG_TOOL_CLASS(JetIRCSafeLabelTool, IJetDecorator)
public:

  /// Enumeration of supported IRC-safe flavour tagging algorithms
  enum class Algo : std::size_t {
    IFN = 0,  ///< Interleaved Flavour Neutralisation
    CMP = 1,  ///< Czakon-Mitov-Poncelet algorithm
    GHS = 2,  ///< Gauld-Huss-Stagnitto flavour dressing
    SDF = 3,  ///< SDFlav algorithm (Marzani et al.)
    AKT = 4,  ///< Anti-kt with net flavour (NOT IRC-safe, for comparison)
    COUNT = 5 ///< Number of algorithms
  };
  static constexpr std::size_t N_ALGOS = static_cast<std::size_t>(Algo::COUNT);

  /// Algorithm configuration constants
  static constexpr double DEFAULT_TRUTH_JET_PT_MIN = 5000.0;
  static constexpr double DEFAULT_RECO_JET_PT_MIN = 10000.0;
  static constexpr double DEFAULT_DR_MAX = 0.3;
  static constexpr double DEFAULT_TRUTH_R = 0.4;
  
  static constexpr double IFN_RFACTOR = 1.0;
  static constexpr double IFN_BETA = 2.0;
  static constexpr double CMP_A = 0.1;
  static constexpr double GHS_ALPHA = 1.0;
  static constexpr double GHS_OMEGA = 0.0;
  static constexpr double GHS_PT_CUT = 5000.0;

  static constexpr int LABEL_B = 5;
  static constexpr int LABEL_C = 4;
  static constexpr int LABEL_LIGHT = 0;
  static constexpr int LABEL_DISABLED = -99;

  /// Constructor
  JetIRCSafeLabelTool(const std::string& name);

  StatusCode initialize() override;

  StatusCode decorate(const xAOD::JetContainer& jets) const override;

protected:
  /// Collect truth particles and cluster them into jets using the enabled algorithms
  std::vector< std::vector<fastjet::PseudoJet> > getJetInputs(
                                                   const xAOD::TruthParticleContainer& parts,
                                                   const xAOD::TruthParticleContainer& label_bs,
                                                   const xAOD::TruthParticleContainer& label_cs) const;

  /// Match truth-level pseudo-jets to reconstructed jets
  std::vector< std::vector<const fastjet::PseudoJet*> > match(
                                                    std::vector<fastjet::PseudoJet>& tagged_jets,
                                                    const xAOD::JetContainer& jets) const;
  
  /// Label name properties
  Gaudi::Property<std::string> m_labelNameIFN{this, "LabelNameIFN", "IRCSafeLabelIFN", 
                                              "Name of the jet label attribute to be added (IFN)"};
  Gaudi::Property<std::string> m_labelNameCMP{this, "LabelNameCMP", "IRCSafeLabelCMP", 
                                              "Name of the jet label attribute to be added (CMP)"};
  Gaudi::Property<std::string> m_labelNameGHS{this, "LabelNameGHS", "IRCSafeLabelGHS", 
                                              "Name of the jet label attribute to be added (GHS)"};
  Gaudi::Property<std::string> m_labelNameSDF{this, "LabelNameSDF", "IRCSafeLabelSDF", 
                                              "Name of the jet label attribute to be added (SDF, Marzani et al.)"};
  Gaudi::Property<std::string> m_labelNameAKT{this, "LabelNameAKT", "IRCSafeLabelAKT", 
                                              "Name of the jet label attribute to be added (anti-kt with net flavour - NOT IRC SAFE)"};
  
  /// Algorithm selection property
  Gaudi::Property<std::vector<std::string>> m_enabledAlgorithms{this, "EnabledAlgorithms", 
                                                                {"IFN", "CMP", "GHS", "SDF", "AKT"}, 
                                                                "Subset of IRCSafe algorithms to run (IFN, CMP, GHS, SDF, AKT)"};
  
  /// Cut and configuration properties
  Gaudi::Property<double> m_truthJetPtMin{this, "TruthJetPtMin", DEFAULT_TRUTH_JET_PT_MIN, 
                                          "Minimum pT of truth jets that are matched to reco for labeling [MeV]"};
  Gaudi::Property<double> m_jetPtMin{this, "JetPtMin", DEFAULT_RECO_JET_PT_MIN, 
                                     "Minimum pT of reco jets to be labeled [MeV]"};
  Gaudi::Property<double> m_drMax{this, "DRMax", DEFAULT_DR_MAX, 
                                  "Maximum deltaR between a particle and jet to be labeled"};
  Gaudi::Property<double> m_truthR{this, "TruthR", DEFAULT_TRUTH_R, 
                                   "Radius with which truth particles will be clustered when determining the flavour "
                                   "(should be equal to the radius of the tagged jets)"};
  
  /// Name of jet label attributes 
  ParticleJetTools::IRCSafeLabelNames m_ircsafelabelnames;
  std::unique_ptr<ParticleJetTools::IRCSafeLabelDecorators> m_ircsafelabeldecs;

  /// Read handles for truth particle collections
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_bottomPartCollectionKey{this, "BParticleCollection", "", "ReadHandleKey for bottomPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_charmPartCollectionKey{this, "CParticleCollection", "", "ReadHandleKey for charmPartCollection"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_outTruthPartKey{this, "TruthParticleCollection", "", "ReadHandleKey of the TruthParticle collection"};

  /// Compact array storing enabled algorithms (true = enabled)
  std::array<bool, N_ALGOS> m_doAlgo{};
  
  /// FastJet configuration cached at initialize to avoid per-event allocations
  std::unique_ptr<fastjet::Selector> m_selectPt;
  std::unique_ptr<fastjet::contrib::FlavRecombiner> m_flavRecombiner;
  std::unique_ptr<fastjet::JetDefinition> m_aktJetDef;
  std::unique_ptr<fastjet::JetDefinition> m_ifnJetDef;
  std::unique_ptr<fastjet::JetDefinition> m_cmpJetDef;

  /// Convenience function to check if an algorithm is enabled
  bool doAlgo(Algo a) const { return m_doAlgo[static_cast<std::size_t>(a)]; }
};

#endif