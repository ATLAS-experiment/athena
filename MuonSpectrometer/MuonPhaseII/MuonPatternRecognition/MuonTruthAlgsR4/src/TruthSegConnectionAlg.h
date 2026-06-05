/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TRUTHSEGCONNECTALG_H
#define MUONTRUTHSEGMENTMAKER_TRUTHSEGCONNECTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuon/MuonSegmentContainer.h"
namespace MuonR4 {
    /*** @brief Algorithm that connects segments stemming from the same 
     *          truth G4 track. The matching is primarly based on the 
     *          commonly shared truth particle. If the particle is skimmed
     *          due to pile-up truth particle surpression, then the 
     *          G4 track id is used for matching */
    class TruthSegConnectionAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
          /** @brief Declare the input truth segment key */
          SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this,"SegmentKey", "MuonTruthSegments"};
          /** @brief Declare the dependency on the truth particle link */
          SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthPartLinkKey{this, "TruthParticleLinkKey",
                                                                                m_segmentKey, "truthParticleLink"};
          /** @brief Declare the decoration written by the algorithm */
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_connectKey{this, "ConnectionLabel",
                                                                           m_segmentKey, "truthSegmentLinks"};
    };
}

#endif