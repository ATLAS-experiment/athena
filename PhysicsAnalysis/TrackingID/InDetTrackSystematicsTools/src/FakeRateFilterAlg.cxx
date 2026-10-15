/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FakeRateFilterAlg.h"
#include <limits>
#include <algorithm>

namespace InDet {

  FakeRateFilterAlg::FakeRateFilterAlg(
      const std::string& name, ISvcLocator* pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode FakeRateFilterAlg::initialize() {
    ATH_CHECK(m_inTrackKey.initialize());
    ATH_CHECK(m_outTrackKey.initialize());
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_decorDeps.initialize(m_inTrackKey, m_outTrackKey));

    CP::SystematicSet systSet;
    if (!m_systematicVariation.value().empty()) {
      systSet = CP::SystematicSet{
        CP::SystematicVariation(m_systematicVariation.value())
      };
    }

    if (!m_inclusiveFilterTool.empty()) {
      ATH_CHECK(m_inclusiveFilterTool.retrieve());
      if (!systSet.empty()) {
        ATH_CHECK(m_inclusiveFilterTool->applySystematicVariation(systSet));
      }
    }

    if (!m_jetFilterTool.empty()) {
      ATH_CHECK(m_jetFilterTool.retrieve());
      if (!systSet.empty()) {
        ATH_CHECK(m_jetFilterTool->applySystematicVariation(systSet));
      }
    }

    if (!m_trackFilterTool.empty()) {
      ATH_CHECK(m_trackFilterTool.retrieve());

      if (!systSet.empty()) {
        ATH_CHECK(m_trackFilterTool->applySystematicVariation(systSet));
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode FakeRateFilterAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);

    auto outTracks = std::make_unique<xAOD::TrackParticleContainer>();
    auto outAux = std::make_unique<xAOD::TrackParticleAuxContainer>();
    outTracks->setStore(outAux.get());

    for (const xAOD::TrackParticle* track : *inTracks) {

      bool pass = true;

      if (!m_trackFilterTool.empty()) {
        pass = m_trackFilterTool->accept(track, jets.ptr());
      }
      else if (!m_jetFilterTool.empty()) {
        pass = m_jetFilterTool->accept(track, jets.ptr());
      }
      else if (!m_inclusiveFilterTool.empty()) {
        pass = m_inclusiveFilterTool->accept(track);
      }

      if (pass) {
        xAOD::TrackParticle* newTrack = new xAOD::TrackParticle();
        outTracks->push_back(newTrack);
        *newTrack = *track;
      }
    }

    SG::WriteHandle<xAOD::TrackParticleContainer> outHandle(m_outTrackKey, ctx);
    ATH_CHECK(outHandle.record(std::move(outTracks), std::move(outAux)));

    return StatusCode::SUCCESS;
  }
} // namespace InDet
