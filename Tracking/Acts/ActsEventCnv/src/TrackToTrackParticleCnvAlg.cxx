/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackToTrackParticleCnvAlg.h"

#include "xAODTracking/TrackParticleAuxContainer.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

namespace ActsTrk {
  StatusCode TrackToTrackParticleCnvAlg::initialize() {
     const std::vector<std::string> supportedStrategies {"BeamLine", "Vertex", "DontRecalculate"};
     bool isAllowedStrategy = false;
     for (const std::string& strategy : supportedStrategies) {
       if (m_perigeeExpression != strategy) continue;
       isAllowedStrategy = true;
       break;
     }
     ATH_MSG_DEBUG("- perigeeExpression: " << m_perigeeExpression.value());
     if (not isAllowedStrategy) {
       ATH_MSG_ERROR("Wrong configuration of the Track to Track Particle Cnv algorithm: perigeeExpression is not supported");
       return StatusCode::FAILURE;
     }

     if (m_perigeeExpression == "BeamLine") {
        m_expression_strategy = expressionStrategy::BeamLine;
     } else if (m_perigeeExpression == "Vertex") {
        m_expression_strategy = expressionStrategy::Vertex;
     } else if (m_perigeeExpression == "DontRecalculate") {
        m_expression_strategy = expressionStrategy::DontRecalculate;
     }

     ATH_CHECK(m_cnvTool.retrieve() );
     ATH_CHECK(m_tracksContainerKey.initialize() );
     ATH_CHECK(m_trackParticlesOutKey.initialize() );
     ATH_CHECK(m_beamSpotKey.initialize(m_expression_strategy == expressionStrategy::BeamLine) );
     ATH_CHECK(m_vertexKey.initialize(m_expression_strategy == expressionStrategy::Vertex) );
     ATH_CHECK(m_decorator_actsTracks.initialize());

     return StatusCode::SUCCESS;
  }

  StatusCode TrackToTrackParticleCnvAlg::execute(const EventContext &ctx) const {
    SG::WriteHandle wh_track_particles( m_trackParticlesOutKey, ctx);
    ATH_CHECK(wh_track_particles.record(std::make_unique<xAOD::TrackParticleContainer>(),
                                        std::make_unique<xAOD::TrackParticleAuxContainer>()));

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> trackLink(m_decorator_actsTracks, ctx);

    xAOD::TrackParticleContainer *track_particles = wh_track_particles.ptr();

    const InDet::BeamSpotData *beamspot_data {nullptr};
    ATH_CHECK(SG::get(beamspot_data, m_beamSpotKey, ctx));
    const xAOD::VertexContainer *vertexContainer {nullptr};
    ATH_CHECK(SG::get(vertexContainer, m_vertexKey, ctx));
    const xAOD::Vertex* primaryVertex {nullptr};

   

    if (m_expression_strategy == expressionStrategy::Vertex) {
      if (vertexContainer->size() == 0) {
        ATH_MSG_ERROR("Retrieved an empty vertex container. This is totally wrong!");
        return StatusCode::FAILURE;
      }

      for(const xAOD::Vertex* vtx : *vertexContainer) {
        if(vtx->vertexType() == xAOD::VxType::PriVtx) {
          primaryVertex = vtx;
          break;
        }
      }

      if (not primaryVertex) {
        ATH_MSG_WARNING("Requested to compute track particles wrt primary vertex, but no primary vertex is found. Using dummy vertex");
        primaryVertex = vertexContainer->front();
      }
    }

    std::size_t nTracks = 0ul;
    std::vector<const ActsTrk::TrackContainer *> trackContainers;
    for (const SG::ReadHandleKey<ActsTrk::TrackContainer>& handleKey : m_tracksContainerKey) {
      ATH_CHECK(SG::get(trackContainers.emplace_back(nullptr), handleKey, ctx));
      nTracks += trackContainers.back()->size();
    }

    // Fast Insertion Trick
    track_particles->reserve(nTracks);
    for (std::size_t i = 0; i<nTracks; ++i) {
      track_particles->push_back( std::make_unique<xAOD::TrackParticle>());
    }

    std::shared_ptr<const Acts::Surface> perigee_surface {nullptr};
    if (m_expression_strategy == expressionStrategy::BeamLine) {
      perigee_surface = makePerigeeSurface(beamspot_data);
    } else if (m_expression_strategy == expressionStrategy::Vertex) {
      perigee_surface = makePerigeeSurface(*primaryVertex);
    }

    std::size_t particleCounter = 0ul;
    for (const ActsTrk::TrackContainer *tracksContainer : trackContainers) {
      for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : *tracksContainer) {
        xAOD::TrackParticle *track_particle = track_particles->at(particleCounter++);
        if (m_expression_strategy == expressionStrategy::DontRecalculate) {
          perigee_surface = track.referenceSurface().getSharedPtr();
        }
        ATH_CHECK(m_cnvTool->convert(*track_particle, ctx, track, *perigee_surface, beamspot_data));

        // add element link to the corresponding track
        trackLink(*track_particle) = ElementLink<ActsTrk::TrackContainer>(*tracksContainer, track.index(), ctx);
        ATH_CHECK( trackLink(*track_particle).isValid() );
      }
    }
    ATH_MSG_DEBUG( "Converted " << nTracks << " acts tracks into " << track_particles->size() << " track particles.");

    return StatusCode::SUCCESS;
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const InDet::BeamSpotData *beamspot_data) {
     // @from TrackToVertex::trackAtBeamline
     Acts::Vector3 beamspot = Amg::Vector3D::Zero();
     float tiltx{0.f},  tilty{0.f};
     if (beamspot_data) {
        beamspot = beamspot_data->beamVtx().position();
        tiltx =  beamspot_data->beamTilt(0);
        tilty =  beamspot_data->beamTilt(1);
     }
     Amg::Transform3D trf = Amg::getTranslate3D(beamspot) * 
                            Amg::getRotateY3D(tilty) *
                            Amg::getRotateX3D(tiltx);
     return Acts::Surface::makeShared<Acts::PerigeeSurface>(trf);
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const xAOD::Vertex& vertex) {
    return Acts::Surface::makeShared<Acts::PerigeeSurface>(Amg::getTranslate3D(vertex.position()));
  }

}
