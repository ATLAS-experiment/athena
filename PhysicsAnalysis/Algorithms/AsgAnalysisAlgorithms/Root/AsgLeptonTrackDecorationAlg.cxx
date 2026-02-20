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
    if (!m_biasingTool.empty())
      ANA_CHECK (m_biasingTool.retrieve());
    if (!m_smearingTool.empty())
      ANA_CHECK (m_smearingTool.retrieve());

    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));

    ANA_CHECK (m_d0sigHandle.initialize(m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_z0sinthetaHandle.initialize(m_systematicsList, m_particlesHandle));
    
    ANA_CHECK (m_d0Handle.initialize(m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_z0Handle.initialize(m_systematicsList, m_particlesHandle));
    ANA_CHECK (m_z0sinthetasigHandle.initialize(m_systematicsList, m_particlesHandle));

    if (!m_biasingTool.empty())
      ANA_CHECK (m_systematicsList.addSystematics (*m_biasingTool));
    if (!m_smearingTool.empty())
      ANA_CHECK (m_systematicsList.addSystematics (*m_smearingTool));
    ANA_CHECK (m_systematicsList.initialize());

    ANA_CHECK (m_eventInfoKey.initialize());
    ANA_CHECK (m_primaryVerticesKey.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode AsgLeptonTrackDecorationAlg ::
  execute ()
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_primaryVerticesKey);
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
      if (!m_biasingTool.empty())
        ANA_CHECK (m_biasingTool->applySystematicVariation (sys));
      if (!m_smearingTool.empty())
        ANA_CHECK (m_smearingTool->applySystematicVariation (sys));
      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (const xAOD::IParticle *particle : *particles)
      {
        float d0sig = -999;
        float d0 = -999;
        float z0 = -999;
        float deltaZ0SinTheta = -999;
        float deltaZ0SinThetasig = -999;

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
          // This deep-copy is not optimal and it would be more efficient to work with shallow-copies of the track container(s)
          xAOD::TrackParticle copyTrack {*track};
          if (!m_biasingTool.empty())
            ANA_CHECK_CORRECTION (m_outOfValidity, copyTrack, m_biasingTool->applyCorrection (copyTrack));
          if (!m_smearingTool.empty())
            ANA_CHECK_CORRECTION (m_outOfValidity, copyTrack, m_smearingTool->applyCorrection (copyTrack));
          d0 = copyTrack.d0();
          d0sig = xAOD::TrackingHelpers::d0significance(&copyTrack,
							eventInfo->beamPosSigmaX(),
							eventInfo->beamPosSigmaY(),
							eventInfo->beamPosSigmaXY());

          z0 = copyTrack.z0();
          const double vertex_z = primaryVertex ? primaryVertex->z() : 0;
          deltaZ0SinTheta = (z0 + copyTrack.vz() - vertex_z) * sin (particle->p4().Theta());
          deltaZ0SinThetasig = xAOD::TrackingHelpers::z0sinthetasignificance(&copyTrack,primaryVertex);
        }

        m_d0Handle.set(*particle,d0,sys);
        m_d0sigHandle.set(*particle, d0sig, sys);
        m_z0Handle.set(*particle,z0,sys);
        m_z0sinthetaHandle.set(*particle, deltaZ0SinTheta, sys);
        m_z0sinthetasigHandle.set(*particle,deltaZ0SinThetasig,sys);
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace
