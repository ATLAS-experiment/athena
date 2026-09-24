/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include <AsgAnalysisAlgorithms/AsgLeptonTrackDecorationAlg.h>
#include "AsgDataHandles/ReadHandle.h"

#include <xAODEgamma/Electron.h>
#include <xAODMuon/Muon.h>
#include <xAODTracking/TrackParticlexAODHelpers.h>

#include <optional>


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
  execute (const EventContext& ctx)
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_primaryVerticesKey, ctx);
    if (!vertices.isValid())
      {
        ANA_MSG_ERROR ("Cannot retrieve primary vertex container " << m_primaryVerticesKey.key());
        return StatusCode::FAILURE;
      }
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
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys, ctx));
      for (const xAOD::IParticle *particle : *particles)
      {
        float d0sig = -999;
        float d0 = -999;
        float z0 = -999;
        float deltaZ0SinTheta = -999;
        float deltaZ0SinThetasig = -999;

        const xAOD::TrackParticle *track {nullptr};
        if (const xAOD::Muon *muon = dynamic_cast<const xAOD::Muon *>(particle)){
          track = muon->trackParticle(xAOD::Muon::TrackParticleType::Primary);
        } else if (const xAOD::Electron *electron = dynamic_cast<const xAOD::Electron *>(particle)){
          track = electron->trackParticle();
        } else {
          ANA_MSG_ERROR ("failed to cast input to electron or muon");
          return StatusCode::FAILURE;
        }

        // This deep-copy is not optimal and it would be more efficient to work with shallow-copies of the track container(s)
        std::optional<xAOD::TrackParticle> correctedTrack;
        if (!m_biasingTool.empty() || !m_smearingTool.empty())
          correctedTrack.emplace (*track);
        if (!m_biasingTool.empty())
          ANA_CHECK_CORRECTION (m_outOfValidity, *correctedTrack, m_biasingTool->applyCorrection (*correctedTrack));
        if (!m_smearingTool.empty())
          ANA_CHECK_CORRECTION (m_outOfValidity, *correctedTrack, m_smearingTool->applyCorrection (*correctedTrack));
        const xAOD::TrackParticle &copyTrack = correctedTrack ? *correctedTrack : *track;
        d0 = copyTrack.d0();
        try {
          d0sig = xAOD::TrackingHelpers::d0significance(&copyTrack,
                                                        eventInfo->beamPosSigmaX(),
                                                        eventInfo->beamPosSigmaY(),
                                                        eventInfo->beamPosSigmaXY());
        } catch (const std::runtime_error &) {
          d0sig = -999;
        }
        z0 = copyTrack.z0();
        const double vertex_z = primaryVertex ? primaryVertex->z() : 0;
        deltaZ0SinTheta = (z0 + copyTrack.vz() - vertex_z) * sin (particle->p4().Theta());
        try {
          deltaZ0SinThetasig = xAOD::TrackingHelpers::z0sinthetasignificance(&copyTrack,primaryVertex);
        } catch (const std::runtime_error &) {
          deltaZ0SinThetasig = -999;
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
