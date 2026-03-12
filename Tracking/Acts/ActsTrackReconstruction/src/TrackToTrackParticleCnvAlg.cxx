/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackToTrackParticleCnvAlg.h"

#include "xAODTracking/TrackParticleAuxContainer.h"

namespace ActsTrk
{

  TrackToTrackParticleCnvAlg::TrackToTrackParticleCnvAlg(const std::string &name,
                                                         ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TrackToTrackParticleCnvAlg::initialize()
  {
     std::vector<std::string> supportedStrategies {"BeamLine", "Vertex"};
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

     if (m_perigeeExpression == "BeamLine") m_expression_strategy = expressionStrategy::BeamLine;
     else if (m_perigeeExpression == "Vertex") m_expression_strategy = expressionStrategy::Vertex;
     else if (m_perigeeExpression == "DontRecalculate") m_expression_strategy = expressionStrategy::DontRecalculate;
     else return StatusCode::FAILURE;

     ATH_CHECK( m_cnvTool.retrieve() );
     ATH_CHECK( m_tracksContainerKey.initialize() );
     ATH_CHECK( m_trackParticlesOutKey.initialize() );
     ATH_CHECK( m_beamSpotKey.initialize(m_expression_strategy == expressionStrategy::BeamLine) );
     ATH_CHECK( m_vertexHandle.initialize(m_expression_strategy == expressionStrategy::Vertex) );

     m_decorator_actsTracks = m_trackParticlesOutKey.key() + "." + m_decorator_actsTracks.key();
     ATH_CHECK(m_decorator_actsTracks.initialize());

     return StatusCode::SUCCESS;
  }

  StatusCode TrackToTrackParticleCnvAlg::execute(const EventContext &ctx) const
  {
    SG::WriteHandle<xAOD::TrackParticleContainer> wh_track_particles( m_trackParticlesOutKey, ctx);
    if (wh_track_particles.record(std::make_unique<xAOD::TrackParticleContainer>(),
                                  std::make_unique<xAOD::TrackParticleAuxContainer>()).isFailure()) {
       ATH_MSG_ERROR("Failed to record track particle container with key " << m_trackParticlesOutKey.key() );
       return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> trackLink(m_decorator_actsTracks, ctx);

    xAOD::TrackParticleContainer *track_particles = wh_track_particles.ptr();

    const InDet::BeamSpotData *beamspot_data {nullptr};
    const xAOD::VertexContainer *vertexContainer {nullptr};
    const xAOD::Vertex* primaryVertex {nullptr};

    if (m_expression_strategy == expressionStrategy::BeamLine) {
      SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle( m_beamSpotKey, ctx );
      ATH_CHECK(beamSpotHandle.isValid());
      beamspot_data = beamSpotHandle.cptr();
    }

    if (m_expression_strategy == expressionStrategy::Vertex) {
      SG::ReadHandle<xAOD::VertexContainer> vertexHandle = SG::makeHandle( m_vertexHandle, ctx );
      ATH_CHECK( vertexHandle.isValid() );
      vertexContainer = vertexHandle.cptr();
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
      SG::ReadHandle<ActsTrk::TrackContainer> handle = SG::makeHandle( handleKey, ctx );
      ATH_CHECK(handle.isValid());
      trackContainers.push_back( handle.cptr() );
      nTracks += trackContainers.back()->size();
    }

    // Fast Insertion Trick
    std::vector<xAOD::TrackParticle*> toAddParticles;
    toAddParticles.reserve(nTracks);
    for (std::size_t i(0); i<nTracks; ++i) {
      toAddParticles.push_back( new xAOD::TrackParticle() );
    }
    track_particles->insert(track_particles->end(),
			    toAddParticles.begin(),
			    toAddParticles.end());

    std::shared_ptr<Acts::PerigeeSurface> perigee_surface {nullptr};
    if (m_expression_strategy == expressionStrategy::BeamLine) {
      perigee_surface = makePerigeeSurface(beamspot_data);
    } else if (m_expression_strategy == expressionStrategy::Vertex) {
      perigee_surface = makePerigeeSurface(*primaryVertex);
    }

    std::size_t particleCounter = 0ul;
    for (const ActsTrk::TrackContainer *tracksContainer : trackContainers) {
      for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : *tracksContainer) {
        xAOD::TrackParticle *track_particle = track_particles->at(particleCounter++);
        ATH_CHECK( m_cnvTool->convert(*track_particle, ctx, track,
                                      perigee_surface.get(), beamspot_data) );

        // add element link to the corresponding track
        trackLink(*track_particle)
          = ElementLink<ActsTrk::TrackContainer>( tracksContainer,
                                                  track.index() );
        ATH_CHECK( (trackLink(*track_particle)).isValid() );
      }
    }
    ATH_MSG_DEBUG( "Converted " << nTracks << " acts tracks into " << track_particles->size() << " track particles.");

    return StatusCode::SUCCESS;
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const InDet::BeamSpotData *beamspot_data) {
     // @from TrackToVertex::trackAtBeamline
     Acts::Vector3 beamspot(0., 0., 0.);
     float tiltx = 0.0;
     float tilty = 0.0;
     if (beamspot_data) {
        beamspot = Acts::Vector3(beamspot_data->beamVtx().position());
        tiltx =  beamspot_data->beamTilt(0);
        tilty =  beamspot_data->beamTilt(1);
     }
     Acts::Translation3 translation(beamspot);
     Acts::Transform3 transform( translation * Acts::RotationMatrix3::Identity() );
     transform *= Acts::AngleAxis3(tilty, Acts::Vector3(0.,1.,0.));
     transform *= Acts::AngleAxis3(tiltx, Acts::Vector3(1.,0.,0.));
     return Acts::Surface::makeShared<Acts::PerigeeSurface>(transform);
  }

  std::shared_ptr<Acts::PerigeeSurface> TrackToTrackParticleCnvAlg::makePerigeeSurface(const xAOD::Vertex& vertex) {
    Acts::Translation3 translation(Acts::Vector3(vertex.position()));
    Acts::Transform3 transform( translation * Acts::RotationMatrix3::Identity() );
    return Acts::Surface::makeShared<Acts::PerigeeSurface>(transform);
  }

}
