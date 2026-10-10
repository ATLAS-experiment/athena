/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRUTHALGSR4_MUONTRUTHMATCHINGALG_H
#define MUONTRUTHALGSR4_MUONTRUTHMATCHINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuon/MuonContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace MuonR4{
    /** @brief Algorithm to match the xAOD::Muon objects to xAOD::TruthParticles 
     *         stemming from the TruthMuons container, if possible. */
    class MuonToTruthAssocAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Declare the dependency on the truth particle container */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
            /** @brief Declare the dependency on the reconstructed segment container to establish the 
             *         reco -> truth link from the segment side */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
            /** @brief Additional track container dependencies. An additional dependency on the `truthParticleLink`
             *         decoration will be declared for each element in the list */
            SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_trkKeys{this, "TrackKeys", {}};
            /** @brief Collector of all the decoration dependencies what are implicitly needed */
            SG::ReadDecorHandleKeyArray<SG::AuxVectorBase> m_decorKeys{this, "DeocrationKeys", {}};
            /** @brief the actual muon container that we want to decorate */
            SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons"};
            /** @brief Explicitly declare the truth particle link decoration. */
            SG::WriteDecorHandleKey<xAOD::MuonContainer> m_truthPartLinkKey{this, "TruthPartLinkKey", m_muonKey, "truthParticleLink"};
    };
}

#endif