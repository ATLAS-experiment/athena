/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
            /** @brief Declare the decorations written by this algorithm */
            SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer> m_hitDecorKeys{this, "HitDecors", {}};
            /** @brief The track summary tool filling the summary state from the associated segments */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" ,""};
    };
}

#endif