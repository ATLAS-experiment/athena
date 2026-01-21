/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TrackToTruthPartAssocAlg_H
#define MUONTRUTHSEGMENTMAKER_TrackToTruthPartAssocAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTracking/TrackParticleContainer.h>

#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/WriteDecorHandleKey.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonRecHelperTools/IMuonEDMHelperSvc.h>

namespace MuonR4{
    /** @brief The TrackToTruthPartAssocAlg matches the reconstructed tracks to truth muons.
      *        The SDO identifiers decorated to the TruthMuon are used for the matching algorithm.
      *        Then, the algorithm navigates from the track particles to the corresponding `Trk::Track`
      *        to collect the hit Identifiers from the TrackStatesOnSurface measurements. The truth particle
      *        with the largest correspondence in hit counts is then associated with the TrackParticle. */
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
            TrkWriteDecorKey_t m_classificationWriteKey{this, "TruthClassificationWriteKey", m_trkKey, "truthClassification"};
            TrkWriteDecorKey_t m_linkWriteKey{this, "TruthLinkWriteKey", m_trkKey, "truthParticleLink"};
            /** @brief Input truth particle keys */
            using TruthReadKey_t = SG::ReadHandleKey<xAOD::TruthParticleContainer>;
            using TruthReadDecorKey_t = SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>;
            using TruthReadDecorKeyArr_t =  SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer>;

            TruthReadKey_t m_truthMuonKey{this, "TruthMuonKey", "MuonTruthParticles"};

            /** @brief List of simHit id decorations to read from the truth particle */
            Gaudi::Property<std::vector<std::string>> m_simHitIds{this, "SimHitIds", {}};
            /** @brief Declaration of the dependency on the simHit decorations */
            TruthReadDecorKeyArr_t m_simHitKeys{this, "TruthSimHitIdKeys", {}};
            /// FIXME ReadDecorHandle should not be used to access
            /// dynamic variables applied by the algorithm which
            /// created the container, instead a
            /// SG::AuxElement::ConstAccessor should be used.
            TruthReadDecorKey_t m_truMuOriginKey{this, "TruthMuonOriginKey", m_truthMuonKey, "truthOrigin"};
            TruthReadDecorKey_t m_truMuTypeKey{this, "TruthMuonTypeKey", m_truthMuonKey, "truthType"};
            TruthReadDecorKey_t m_truMuClassificationKey{this, "TruthMuonClassificationKey", m_truthMuonKey, "truthClassification"};

    };
}

#endif
