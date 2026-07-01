/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASG_ANALYSIS_ALGORITHMS__JET_TRIGGER_DECORATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__JET_TRIGGER_DECORATOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <AsgTools/PropertyWrapper.h>

#include <xAODJet/JetContainer.h>
#include <xAODTrigger/JetRoIContainer.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <TrigBtagEmulationTool/ITrigBtagEmulationTool.h>


namespace CP
{
  class JetTriggerDecoratorAlg final : public EL::AnaAlgorithm
  {
  public:

    JetTriggerDecoratorAlg(const std::string &name,
			   ISvcLocator *svcLoc = nullptr);

    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;

  private:

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    //// Input jets and selection
    CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
	this, "jets", "", "the jet container to use"};

    SG::ReadHandleKey<xAOD::JetRoIContainer> m_L1JetsInKey{
      this, "L1Jets", "LVL1JetRoIs", "L1 jet container"
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
    Gaudi::Property<float> m_dR{this, "dR", 0.4};

    ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool;
    ToolHandle<Trig::ITrigBtagEmulationTool> m_emulationTool
      {this, "trigEmulationTool", "", "Jet trigger emulation tool, to be used for Run 2"};

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
    CP::SysWriteDecorHandle<float> m_HLTDR_decor {this, "HLTDR", "", "HLT-matched thresholds"};
    CP::SysWriteDecorHandle<std::vector<int>> m_HLTThreshold_decor {this, "HLTThreshold", "", "HLT-matched thresholds"};

    bool isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const;
    
  };
}

#endif
