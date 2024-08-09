//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//

// Local include(s):
#include "TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h"

namespace CP {

    TrackParticleMergerAlg::TrackParticleMergerAlg( const std::string& name,
                                                    ISvcLocator* svcLoc )
    : EL::AnaReentrantAlgorithm( name, svcLoc ) {
    }

    StatusCode TrackParticleMergerAlg::initialize() {

        ATH_MSG_DEBUG("Initializing TrackParticleMergerAlg");
        ATH_CHECK( m_inputTrackParticleLocations.initialize() );

        ATH_CHECK( m_outputTrackParticleLocationCopy.initialize(!m_createViewCollection.value()) );
        ATH_CHECK( m_outputTrackParticleLocationView.initialize(m_createViewCollection.value()) );

        ATH_CHECK( m_requiredDecorations.initialize(!m_createViewCollection.value()) );

        return StatusCode::SUCCESS;
    }

    StatusCode TrackParticleMergerAlg::execute(const EventContext& ctx) const {

        // Setup containers for output, to avoid const conversions setup two different kind of containers
        auto outputViewCol = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer>>(SG::VIEW_ELEMENTS);
        auto outputCol = std::make_unique<xAOD::TrackParticleContainer>();

        std::unique_ptr<xAOD::TrackParticleAuxContainer> outputAuxCol;
        if(!m_createViewCollection) {
            outputAuxCol = std::make_unique<xAOD::TrackParticleAuxContainer>();
            outputCol->setStore(outputAuxCol.get());
        }

        auto readHandles = m_inputTrackParticleLocations.makeHandles(ctx);
        for (auto& readHandle : readHandles) {
            for (const xAOD::TrackParticle* tp : *readHandle) {
                if (m_createViewCollection) {
                    outputViewCol->push_back(tp);
                } else {
                    xAOD::TrackParticle* newTp = new xAOD::TrackParticle();
                    outputCol->push_back(newTp);
                    *newTp = *tp;
                }
           }
        }

        if (m_createViewCollection) {
            SG::WriteHandle<ConstDataVector <xAOD::TrackParticleContainer> > outputTrackParticlesView(m_outputTrackParticleLocationView, ctx);
            ATH_CHECK( outputTrackParticlesView.record(std::move(outputViewCol)) );
        }
        else {
            SG::WriteHandle<xAOD::TrackParticleContainer> outputTrackParticlesCopy(m_outputTrackParticleLocationCopy, ctx);
            ATH_CHECK(outputTrackParticlesCopy.record(std::move(outputCol), std::move(outputAuxCol)));
        }

        return StatusCode::SUCCESS;
    }

} // namespace CP
