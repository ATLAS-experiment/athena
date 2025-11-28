/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler


#ifndef F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_MATCHING_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_MATCHING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODJet/JetContainer.h>
#include <TrigDecisionTool/TrigDecisionTool.h>

namespace CP
{
    /// \brief an algorithm for calling \ref IBTaggingEfficiencyTool including b-jet trigger SF

    class BTaggingTriggerMatchingAlg final : public EL::AnaAlgorithm
    {
        /// \brief the standard constructor
    public:
        using EL::AnaAlgorithm::AnaAlgorithm;

        BTaggingTriggerMatchingAlg(const std::string &name,
                    ISvcLocator *svcLoc = nullptr);
        StatusCode initialize () override;
        StatusCode execute () override;

    private:
        ToolHandle<Trig::TrigDecisionTool> m_trigDecTool;
        Gaudi::Property<std::string> m_trigger {this, "trigger", "",
        "the trigger path to consider"};
        Gaudi::Property<bool> m_useRun3TriggerEDM {this, "useRun3TriggerEDM", true,
        "is the Run-3 trigger EDM available" };
        Gaudi::Property<float> m_btagThreshold {this, "btagThreshold", -1.,
        "b-tag trigger cut, only used with Run 2 trigger EDM"};
        Gaudi::Property<float> m_etamax {this, "etaMax", 2.4, 
        "eta max cut from trigger chain"};
        Gaudi::Property<std::vector<std::string>> m_ftagRun3TriggerDecoNames {this, "ftagRun3TriggerDecoNames", {},
        "list of ftagging trigger decoration names to check for Run-3 trigger EDM"};

        /// \brief the systematics list we run
        SysListHandle m_systematicsList {this};

        /// \brief the jet collection we run on
        SysReadHandle<xAOD::JetContainer> m_jetHandle {
        this, "jets", "Jets", "the jet collection to run on"};

        /// \brief the preselection we apply to our input
        SysReadSelectionHandle m_preselection {
        this, "preselection", "", "the preselection to apply"};

        /// \brief the decoration for the b-tagging scale factor
        SysWriteDecorHandle<char> m_matchingDecoration {
        this, "matchingDecoration", "", "the decoration for offline jet matched to HLT"};
        SysWriteDecorHandle<char> m_bTagMatchingDecoration {
        this, "bTagMatchingDecoration", "", "the decoration for offline jet  matched to HLT b-tag"};

        StatusCode passTriggerBtag(const xAOD::Jet* jet,
                    const std::map<const xAOD::Jet*, const xAOD::Jet*>& matchedOfflineOnlineJets,
                    bool& pass, bool& matched) const;

        bool isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const;
        
        StatusCode getBtagScore(const xAOD::IParticle* jet, double& hlt_bscore) const;

        StatusCode hasBTagDeco(const xAOD::IParticle* jet, bool& hasBtagDeco) const;
        
        SG::ReadHandleKey<xAOD::JetContainer> m_bjetInput {this, "HLT_BJets", 
        "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets", "Input b-Jet Collection Key, retrieved from reconstructed jets"};   

        std::vector<SG::ConstAccessor<float>> m_ftagRun3TriggerDecorAccessors;
        
    }; // class BTaggingTriggerMatchingAlg
} // namespace CP

#endif