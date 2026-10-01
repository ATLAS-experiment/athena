/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASG_ANALYSIS_ALGORITHMS__JET_TRIGGER_DECORATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__JET_TRIGGER_DECORATOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTools/ToolHandle.h>
#include <JetAnalysisAlgorithms/JfexThresholdTable.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <TrigBtagEmulationTool/ITrigBtagEmulationTool.h>
#include <TrigConfInterfaces/ITrigConfigTool.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <xAODJet/JetContainer.h>
#include <xAODTrigger/JetRoIContainer.h>
#include <xAODTrigger/jFexSRJetRoIContainer.h>

namespace CP
{
  /// @brief an algorithm decorating jets with the kinematics, ΔR and
  /// thresholds of the L1 RoI and HLT jet matched to them for a given
  /// trigger chain

  class JetTriggerDecoratorAlg final : public EL::AnaAlgorithm
  {
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;

   private:
    /// @brief the systematics list we run
    SysListHandle m_systematicsList {this};

    //// Input jets and selection
    CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
	this, "jets", "", "the jet container to use"};

    SG::ReadHandleKey<xAOD::JetRoIContainer> m_L1JetsInKey{
      this, "L1Jets", "LVL1JetRoIs", "Legacy L1Calo jet RoI container"
    };
    // Phase-I L1Calo jFEX SR jet RoI container (data 2024+ / mc23e).
    SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_L1JetsPhaseIInKey{
      this, "L1JetsPhaseI", "L1_jFexSRJetRoI",
      "Phase-I L1Calo jFEX SR jet RoI container"
    };
    SG::ReadHandleKey<xAOD::JetContainer> m_HLTJetsInKey {
      this, "HLTJets", "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets",
	"HLT jet container"
    };

    Gaudi::Property<std::string> m_trigger{
      this, "trigger", {}, "trigger to match"
    };
    // To add missing HLT jets -- only for buggy triggers
    // These extra jets are added because of bug in trigger navigation
    // Will be removed once bug fixed at DAOD level
    Gaudi::Property<std::vector<std::string>> m_triggerNavBug{
      this, "triggerBugList", {}, "List of buggy triggers"
    };
    Gaudi::Property<float> m_l1dR{this, "l1dR_cut", 0.4,
        "ΔR cone for L1 (jFEX) matching"};
    Gaudi::Property<float> m_hltDR{this, "hltDR_cut", 0.4,
        "ΔR cone for HLT matching"};

    // PublicToolHandle: TrigDecisionTool is a shared singleton; a private
    // ToolHandle would try to instantiate a duplicate and fail.
#ifndef XAOD_STANDALONE
    // For AthAnalysis and Athena, PublicToolHandle exist
    PublicToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool{
        this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool",
        "trigger decision tool"};
#else
    // For AnalysisBase use ToolHandle as PublicToolHandle is not available
    ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool{
        this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool",
        "trigger decision tool"};
#endif
    ToolHandle<Trig::ITrigBtagEmulationTool> m_emulationTool
      {this, "trigEmulationTool", "", "Jet trigger emulation tool, to be used for Run 2"};

    Gaudi::Property<bool> m_usePhaseIL1{
      this, "usePhaseIL1", false,
      "If true, use Phase-I L1Calo jFEX SR jet RoI container for L1 matching."
    };

    Gaudi::Property<std::string> m_l1ThresholdType{
      this, "l1ThresholdType", "jJ",
      "L1 threshold type for jFEX bit→name lookup (Phase-I L1 only)."
    };

    ToolHandle<TrigConf::ITrigConfigTool> m_trigConfigTool{
      this, "TrigConfigTool", "TrigConf::xAODConfigTool/xAODConfigTool",
      "Trigger configuration tool (Phase-I L1 menu access)"
    };

    // Phase-I L1 bit→name table, built lazily; rebuilt on menu-name change.
    JfexThresholdTable m_jfexThresholdTable;

    /// @brief a "j" leg of the chain, parsed once in initialize()
    struct JetLeg
    {
      int index = 0;       ///< leg index within the chain
      int threshold = 0;   ///< leg threshold (the gsc threshold for gsc legs)
      std::string name;    ///< leg name (key of the Run 2 emulation results)
    };
    std::vector<JetLeg> m_jetLegs;

    /// @brief whether m_trigger is in m_triggerNavBug
    bool m_isNavBugTrigger {false};

    /// @brief whether we already warned about m_trigger missing from the menu
    bool m_warnedMissingChain {false};


    Gaudi::Property<bool> m_doL1Matching{this, "doL1Matching", false,
	"do trigger L1 matching?" };
    CP::SysWriteDecorHandle<float> m_L1Et_decor {this, "L1Et", "", "L1-matched Et"};
    CP::SysWriteDecorHandle<float> m_L1Eta_decor {this, "L1Eta", "", "L1-matched eta"};
    CP::SysWriteDecorHandle<float> m_L1Phi_decor {this, "L1Phi", "", "L1-matched phi"};
    CP::SysWriteDecorHandle<float> m_L1DR_decor {this, "L1DR", "", "L1-matched DR"};
    CP::SysWriteDecorHandle<std::vector<int>> m_L1Threshold_decor {this, "L1Threshold", "", "L1-matched thresholds"};
    
    Gaudi::Property<bool> m_doHLTMatching{this, "doHLTMatching", false,
	"do trigger HLT matching?" };
    Gaudi::Property<bool> m_useEmulationTool{this, "useEmulationTool", false,
	"use HLT jet trigger emulation tool" };
    CP::SysWriteDecorHandle<float> m_HLTPt_decor {this, "HLTPt", "", "HLT-matched pt"};
    CP::SysWriteDecorHandle<float> m_HLTEta_decor {this, "HLTEta", "", "HLT-matched eta"};
    CP::SysWriteDecorHandle<float> m_HLTPhi_decor {this, "HLTPhi", "", "HLT-matched phi"};
    CP::SysWriteDecorHandle<float> m_HLTDR_decor{this, "HLTDR", "",
                                                 "HLT-matched dR"};
    CP::SysWriteDecorHandle<std::vector<int>> m_HLTThreshold_decor {this, "HLTThreshold", "", "HLT-matched thresholds"};

    bool isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const;
    
  };
}

#endif
