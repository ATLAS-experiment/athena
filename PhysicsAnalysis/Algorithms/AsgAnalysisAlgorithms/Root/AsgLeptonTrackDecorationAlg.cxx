/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include <AsgAnalysisAlgorithms/AsgLeptonTrackDecorationAlg.h>
#include "AsgDataHandles/ReadHandle.h"

#include <xAODEgamma/Electron.h>
#include <xAODMuon/Muon.h>
#include <xAODTracking/TrackParticlexAODHelpers.h>


namespace CP
{

  StatusCode AsgLeptonTrackDecorationAlg ::
  initialize ()
  {

    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));

    ANA_CHECK (m_d0sigHandle.initialize(m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_z0sinthetaHandle.initialize(m_systematicsList, m_particlesHandle));
    
    ANA_CHECK (m_systematicsList.initialize());

    ANA_CHECK (m_eventInfoKey.initialize());
    ANA_CHECK (m_primaryVerticesKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode AsgLeptonTrackDecorationAlg ::
  execute (const EventContext &ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_primaryVerticesKey, ctx);
    const xAOD::Vertex *primaryVertex {nullptr};

    for (const xAOD::Vertex *vertex : *vertices)
    {
      if (vertex->vertexType() == xAOD::VxType::PriVtx)
      {
        if (primaryVertex == nullptr)
        {
          primaryVertex = vertex;
          break;
        }
      }
    }

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (const xAOD::IParticle *particle : *particles)
      {
        float d0sig = -999;
        float deltaZ0SinTheta = -999;

        const xAOD::TrackParticle *track {nullptr};
        if (const xAOD::Muon *muon = dynamic_cast<const xAOD::Muon *>(particle)){
          track = muon->primaryTrackParticle();
        } else if (const xAOD::Electron *electron = dynamic_cast<const xAOD::Electron *>(particle)){
          track = electron->trackParticle();
        } else {
          ANA_MSG_ERROR ("failed to cast input to electron or muon");
          return StatusCode::FAILURE;
        }

        if (track != nullptr) {
          d0sig = xAOD::TrackingHelpers::d0significance(track,
							eventInfo->beamPosSigmaX(),
							eventInfo->beamPosSigmaY(),
							eventInfo->beamPosSigmaXY());

          const double vertex_z = primaryVertex ? primaryVertex->z() : 0;
          deltaZ0SinTheta = (track->z0() + track->vz() - vertex_z) * sin (particle->p4().Theta());
        }

        m_d0sigHandle.set(*particle, d0sig, sys);
        m_z0sinthetaHandle.set(*particle, deltaZ0SinTheta, sys);
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace
