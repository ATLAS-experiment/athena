/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONFASTRECOALGS_MUONFASTSABUILDERALG__H
#define MUONR4_MUONFASTRECOALGS_MUONFASTSABUILDERALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"
#include "MuonFastRecoEvent/GlobalPattern.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainerR4.h"
#include "xAODMuonViews/FillContainer.h"
#include "xAODMuonViews/ContainerDecorator.h"

namespace MuonR4 {
    /// @brief Algorithm handling the fast building of standalone muon candidates.
    /// 
    /// This algorithm processes the segments found in previous steps to build 
    /// standalone muon candidates, including the momentum estimation, as
    /// final step of the Phase-2 fast reconstruction. The algorithm consumes the 
    /// xAOD::Muonsegments and produces xAOD::Muons, decorated with links to the
    /// original segments. 

    class MuonFastSABuilderAlg: public AthReentrantAlgorithm{
        public:

            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~MuonFastSABuilderAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /** @brief Define the muon container type */
            using MuonCont_t = xAOD::FillContainer<xAOD::MuonContainer, xAOD::MuonAuxContainerR4>;
            /** @brief Data ship containing the muon container and the covariance decorator */
            struct MuonDataShip {
                /** @brief The muon container */
                MuonCont_t muonContainer;
                /** @brief The Q/P covariance decorator */
                xAOD::ContainerDecorator<xAOD::MuonContainer, float> dec_qOverPCov{};
            };
            /** @brief Given the muon segments, it builds the standalone muon candidate, 
             *         estimating the momentum and filling the output container.
             *  @param ctx: Event context
             *  @param segments: Vector of muon segments
             *  @param outMuonData: Data ship containing the output muon container and decorators
             *  @return: Pointer to the built muon candidate if successful, otherwise nullptr */
            xAOD::Muon* buildMuonCandidate(const EventContext& ctx,
                                           const GlobalPattern& pattern,
                                           std::span<const xAOD::MuonSegment* const> segments,
                                           MuonDataShip& outMuonData) const;

            /** @brief Write handle key for the output muon candidates */ 
            SG::WriteHandleKey<xAOD::MuonContainer> m_outMuons{this, "OutMuons", "FastRecoSAMuons", "Output muon container"};
            /** @brief Muon decoration of the Q/P covariance */
            SG::WriteDecorHandleKey<xAOD::MuonContainer> m_qOverPCovKey{this, "QOverPCovKey", m_outMuons, "QOverPCov", "Q/P covariance for the muons"};
            /** @brief Read handle key for the input segments */ 
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_inSegments{this, "InSegments", "FastMuonSegments"};

            /** @brief The track seeding tool to construct the seed candidates and to estimate the initial parameters */
            ToolHandle<ITrackSeedingTool> m_seedingTool{this, "SeedingTool", ""};
    };
}

#endif