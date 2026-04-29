/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackSmearingAlg.h"
#include "xAODCore/ShallowCopy.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace InDet {
StatusCode TrackSmearingAlg::initialize()
{
  ATH_CHECK(m_smearingTool.retrieve());
  if (!m_biasingTool.empty())
    ATH_CHECK(m_biasingTool.retrieve());

  if (!m_systematicVariation.value().empty())
    m_systSet = CP::SystematicSet({CP::SystematicVariation(m_systematicVariation)});

  // Pre-register the systematic set in each tool's cache so the
  // reentrant applyCorrection(track, syst) overload can look it up.
  ATH_CHECK(m_smearingTool->applySystematicVariation(m_systSet));
  if (!m_biasingTool.empty())
    ATH_CHECK(m_biasingTool->applySystematicVariation(m_systSet));

  ATH_CHECK(m_inTrackKey.initialize());
  ATH_CHECK(m_outTrackKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TrackSmearingAlg::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
  ATH_CHECK(inTracks.isValid());

  auto [outTracks, outAux] =
      xAOD::shallowCopyContainer(*inTracks, ctx);

  for (xAOD::TrackParticle* trk : *outTracks) {
    if (m_smearingTool->applyCorrection(*trk, m_systSet)
            == CP::CorrectionCode::Error) {
      ATH_MSG_ERROR("Could not apply InDetTrackSmearingTool.");
      return StatusCode::FAILURE;
    }
    if (!m_biasingTool.empty()) {
      if (m_biasingTool->applyCorrection(*trk, m_systSet)
              == CP::CorrectionCode::Error) {
        ATH_MSG_ERROR("Could not apply InDetTrackBiasingTool.");
        return StatusCode::FAILURE;
      }
    }
  }

  SG::WriteHandle<xAOD::TrackParticleContainer> outHandle(m_outTrackKey, ctx);
  ATH_CHECK(outHandle.record(std::move(outTracks), std::move(outAux)));
  return StatusCode::SUCCESS;
}

} // namespace InDet
