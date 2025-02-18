/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TrackToTruthPartAssocAlg_H
#define MUONTRUTHSEGMENTMAKER_TrackToTruthPartAssocAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTracking/TrackParticleContainer.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/WriteDecorHandleKey.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonRecHelperTools/IMuonEDMHelperSvc.h>

namespace MuonR4{
    /** @brief The TrackToTruthPartAssocAlg matches the reconstructed tracks to truth muons.
     *         As baseline, the 
     */
    class TrackToTruthPartAssocAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            
        
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Helper service to handle the Identifiers of measurements */
            ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "EdmHelperSvc", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc"};
        

            using TrkReadKey_t = SG::ReadHandleKey<xAOD::TrackParticleContainer>;
            using TrkWriteDecorKey_t = SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>;

            TrkReadKey_t m_trkKey{this, "TrackCollection", "MSTrks"};
            /** @brief Decorations to be written to the TrackParticle truthOrigin/truthType/truthParticleLink */
            TrkWriteDecorKey_t m_originWriteKey{this, "TruthOriginWriteKey", m_trkKey, "truthOrigin"};
            TrkWriteDecorKey_t m_typeWriteKey{this, "TruthTypeWriteKey", m_trkKey, "truthType"};
            TrkWriteDecorKey_t m_linkWriteKey{this, "TruthLinkWriteKey", m_trkKey, "truthParticleLink"};
            /** @brief Input truth particle keys */
            using TruthReadKey_t = SG::ReadHandleKey<xAOD::TruthParticleContainer>;
            using TruthDecor_t = SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>;
            using TruthDecorArr_t =  SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer>;

            TruthReadKey_t m_truthMuonKey{this, "TruthMuonKey", "MuonTruthParticles"};

            /** @brief List of simHit id decorations to read from the truth particle */
            Gaudi::Property<std::vector<std::string>> m_simHitIds{this, "SimHitIds", {}};
            /** @brief Declaration of the dependency on the simHit decorations */
            TruthDecorArr_t m_simHitKeys{this, "TruthSimHitIdKeys", {}};

            TruthDecor_t m_truMuOriginKey{this, "TruthMuonOriginKey", m_truthMuonKey, "truthOrigin"};
            TruthDecor_t m_truMuTypeKey{this, "TruthMuonTypeKey", m_truthMuonKey, "truthType"};

    };
}

#endif