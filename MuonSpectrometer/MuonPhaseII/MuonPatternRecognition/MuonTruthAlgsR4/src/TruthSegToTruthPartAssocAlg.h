/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TruthSegToTruthPartAssocAlg_H
#define MUONTRUTHSEGMENTMAKER_TruthSegToTruthPartAssocAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/WriteDecorHandleKey.h>

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


namespace MuonR4{
    class TruthSegToTruthPartAssocAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Key to the truth particle container to associate */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
            /** @brief List of simHit id decorations to read from the truth particle */
            Gaudi::Property<std::vector<std::string>> m_simHitIds{this, "SimHitIds", {}};
            /** @brief Declaration of the dependency on the simHit decorations */
            SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer> m_simHitKeys{this, "TruthSimHitIdKeys", {}};
            /** @brief Declaration of the segmentLink to the truth particle */
            SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_segLinkKey{this, "SegmentToPartKey", m_truthKey, "truthSegLinks"};
            /** @brief Key to the truth segment container to associate */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "TruthSegmentsR4"};
            /** @brief Key of the truthParticleLink decorated onto the segment */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_truthLinkKey{this, "TruthToPartKey", m_segmentKey, "truthParticleLink"};

    };
}


#endif