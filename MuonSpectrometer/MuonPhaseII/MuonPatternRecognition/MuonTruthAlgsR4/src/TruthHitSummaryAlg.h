/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_TRUTHHITSUMMARYALG_H
#define MUONTRUTHALGSR4_TRUTHHITSUMMARYALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTruth/TruthParticleContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"
namespace MuonR4{
    /*** @brief Algorithm that takes the hit summaries from the truth segments
     *          associated to the muon truth particles and transforms them into
     *          a TrackSummary object. The track summary object is then decorated 
     *          onto the TruthParticle. @note The summary decorations are not locked
     *          and need to be locked by the `LockDecorations` algorithm. Its scheduling
     *          needs to be explicitly configured */
    class TruthHitSummaryAlg : public AthReentrantAlgorithm {
        public:
           using AthReentrantAlgorithm::AthReentrantAlgorithm;
           virtual StatusCode initialize() override final;
           virtual StatusCode execute(const EventContext& ctx) const override final;

        private:
            /** @brief input truth particle container */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_readKey{this, "ReadKey", "MuonTruthParticles"};
            /** @brief Dependency on the truth -> segment decoration */
            SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_segLinkKey{this, "SegLinkKey", m_readKey, "truthSegmentLinks"};
            /** @brief The track summary tool filling the summary state from the associated segments */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" ,""};
    };
}

#endif