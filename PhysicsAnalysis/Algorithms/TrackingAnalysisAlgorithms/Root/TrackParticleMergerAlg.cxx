//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>

namespace CP {

    StatusCode TrackParticleMergerAlg::initialize() {

        ATH_MSG_DEBUG("Initializing TrackParticleMergerAlg");
        ATH_CHECK( m_inputTrackParticleLocations.initialize() );

        ATH_CHECK( m_outputTrackParticleLocationCopy.initialize(!m_createViewCollection.value()) );
        ATH_CHECK( m_outputTrackParticleLocationView.initialize(m_createViewCollection.value()) );

        ATH_CHECK( m_requiredDecorations.initialize(!m_createViewCollection.value()) );

        return StatusCode::SUCCESS;
    }

    StatusCode TrackParticleMergerAlg::execute(const EventContext& ctx) const {

        auto readHandles = m_inputTrackParticleLocations.makeHandles(ctx);
        std::size_t nTracks = 0;
        for (auto& readHandle : readHandles) {
            nTracks += readHandle->size();
        }

        if (m_createViewCollection) {
            // View collection pointing to the input tracks
            auto outputViewCol = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);
            outputViewCol->reserve(nTracks);
            for (auto& readHandle : readHandles) {
                for (const xAOD::TrackParticle* tp : *readHandle) {
                    outputViewCol->push_back(tp);
                }
            }

            SG::WriteHandle<ConstDataVector <xAOD::TrackParticleContainer> > outputTrackParticlesView(m_outputTrackParticleLocationView, ctx);
            ATH_CHECK( outputTrackParticlesView.record(std::move(outputViewCol)) );
        } else {
            // Deep copy of the input tracks
            auto outputCol = std::make_unique<xAOD::TrackParticleContainer>();
            auto outputAuxCol = std::make_unique<xAOD::TrackParticleAuxContainer>();
            outputCol->setStore(outputAuxCol.get());
            outputCol->reserve(nTracks);
            for (auto& readHandle : readHandles) {
                for (const xAOD::TrackParticle* tp : *readHandle) {
                    xAOD::TrackParticle* newTp = outputCol->push_back(std::make_unique<xAOD::TrackParticle>());
                    *newTp = *tp;
                }
            }

            SG::WriteHandle<xAOD::TrackParticleContainer> outputTrackParticlesCopy(m_outputTrackParticleLocationCopy, ctx);
            ATH_CHECK( outputTrackParticlesCopy.record(std::move(outputCol), std::move(outputAuxCol)) );
        }

        return StatusCode::SUCCESS;
    }

} // namespace CP
