/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_DIMUONTAGGINGALG_H
#define DERIVATIONFRAMEWORK_DIMUONTAGGINGALG_H


// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

// xAOD header files
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticleContainer.h"


namespace DerivationFramework {

    class DiMuonTaggingAlg : public AthReentrantAlgorithm {
    public:
        /** Constructor with parameters */
        DiMuonTaggingAlg(const std::string& name, ISvcLocator* pSvcLocator);

        /** Destructor */
        virtual ~DiMuonTaggingAlg() = default;
        // Athena algtool's Hooks
        StatusCode initialize() override;

        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        using TrackPassDecor = SG::WriteDecorHandle<xAOD::TrackParticleContainer, bool>;
        /// Returns true of the pointer is valid and also whether the pt and absEta are above and below the thresholds, respectively
        bool passKinematicCuts(const xAOD::IParticle* mu, const float ptMin, const float absEtaMax) const;

        bool passMuonCuts(const xAOD::Muon* muon, const float ptMin, const float absEtaMax, const bool applyQuality) const;

        bool passTrigger(const xAOD::Muon* muon, const std::vector<std::string>& trigList) const;

        template <class probe_type> bool muonPairCheck(const xAOD::Muon* mu1, const probe_type* mu2) const;

        void maskNearbyIDtracks(const xAOD::IParticle* mu, TrackPassDecor& decor) const;
        
        
        Gaudi::Property<bool> m_applyTrig{this, "applyTrigger", true,
                                         "Flag deciding whether the trigger selection should be applied"};
        
        /// List of triggers in which at least one muon in the pair needs to pass
        Gaudi::Property<std::vector<std::string>> m_orTrigs{this, "OrTrigs", {}};
        /// List of trigger in which both muons need to pass the selection
        Gaudi::Property<std::vector<std::string>> m_andTrigs{this, "AndTrigs", {}};

        
        ToolHandle<Trig::IMatchingTool> m_matchingTool{this, "TrigMatchingTool", "",
                                                       "Tool to access the trigger decision"};


        Gaudi::Property<float> m_triggerMatchDeltaR{this, "TriggerMatchDeltaR", 0.1};


        SG::ReadHandleKey<xAOD::MuonContainer> m_muonSGKey{this, "MuonContainerKey", "Muons"};

        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackSGKey{this, "TrackContainerKey", "InDetTrackParticles"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthSGKey{this, "TruthKey", ""};

        /// Keys to whitelist the muons & tracks needed for MCP studies to output
        SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonKeepKey{this, "MuonKeepKey", m_muonSGKey, "", 
                                "Key to whitelist the muon for writeout. Will be overwritten by BranchPrefix property"};
        SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trkKeepKey{this, "TrackKeepKey", m_trackSGKey, "",  
                                "Key to whitelist the tracks for writeout. Will be overwritten by BranchPreFix property"};
        /// Event Decision Key
        SG::WriteHandleKey<int> m_skimmingKey{this, "SkimmingKey", "", "Set via BranchPreFixProperty + DIMU_pass"};

        /// Selection tool to filter muons with poor quality
        ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "SelectionTool", ""};
        /// Pt cut on the primary muon in the event
        Gaudi::Property<float> m_mu1PtMin{this, "Mu1PtMin", -1};
        /// Eta cut on the primary muon in the event
        Gaudi::Property<float> m_mu1AbsEtaMax{this, "Mu1AbsEtaMax", 10};
        /// Flag to toggle whether selection working point should be applied on the first muon
        Gaudi::Property<bool>  m_applyQualityMu1{this, "Mu1RequireQual", false};

        /// Pt cut on the second muon in the event
        Gaudi::Property<float> m_mu2PtMin{this, "Mu2PtMin", -1};
        /// Eta cut on the second muon in the event
        Gaudi::Property<float> m_mu2AbsEtaMax{this, "Mu2AbsEtaMax", 10};
        /// Flag to toggle whether the selection working point should be applied on the second muon
        Gaudi::Property<bool>  m_applyQualityMu2{this, "Mu2RequireQual", false};
        
        /// Flag whether it's MC
        Gaudi::Property<bool> m_isMC{this, "isMC", false};
        /// Run the analysis with a Muon <--> Track pair 
        ///  --- Muon has to hold criteria 1 and track to satisfy kinematic criteria 2
        Gaudi::Property<bool> m_useTrackProbe{this, "UseTrackProbe", false};
        /// Name of the ouput selection flag
        Gaudi::Property<std::string> m_br_prefix{this, "BranchPrefix", ""};

        /// Both particles have to be of opposite charge
        Gaudi::Property<bool> m_requireOS{this, "OppositeCharge", true};
        /// Require a minimal delta Phi between the two selected candidates
        Gaudi::Property<float> m_dPhiMin{this, "PairDPhiMin", -1};
        /// Lower invariant mass selection window
        Gaudi::Property<float> m_invariantMassLow{this, "InvariantMassLow", 2.0 * Gaudi::Units::GeV};
        /// Upper invariant mass selection window (Negative number refers to disable)
        Gaudi::Property<float> m_invariantMassHigh{this, "InvariantMassHigh",-1.*Gaudi::Units::GeV};

        Gaudi::Property<float> m_thinningConeSize{this, "IDTrackThinningConeSize", 0.4};

        float m_invariantMassLow2{0};
        float m_invariantMassHigh2{0};
        float m_thinningConeSize2{0};
    };
}  // namespace DerivationFramework
#endif  // DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
