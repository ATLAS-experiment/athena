/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNNAuxTaskDecoratorAlg.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODBTagging/BTagging.h"

#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include <vector>
#include <map>

namespace FlavorTagDiscriminants {
  GNNAuxTaskDecoratorAlg::GNNAuxTaskDecoratorAlg(
    const std::string& name, ISvcLocator* svcloc):
    AthReentrantAlgorithm(name, svcloc)
  {
  }
  StatusCode GNNAuxTaskDecoratorAlg::initialize() {
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_trackContainerKey.initialize());

    ATH_CHECK(m_trackLinksKey.initialize());

    m_trackAuxTasksDecorKeys.reserve(m_trackAuxTasks.size());
    m_readDecorKeys.reserve(m_trackAuxTasks.size());
    for (const auto& [jetDecor, trackDecor] : m_trackAuxTasks) {
      m_trackAuxTasksDecorKeys.emplace_back(m_trackContainerKey, trackDecor);
      m_readDecorKeys.emplace_back(m_jetContainerKey, jetDecor);
    }
    ATH_CHECK(m_trackAuxTasksDecorKeys.initialize());
    ATH_CHECK(m_readDecorKeys.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode GNNAuxTaskDecoratorAlg::execute(const EventContext& ctx) const {
    // define the required WriteDecorHandle objects
    std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer, char>> track_wdhs;
    for (const auto& wdhk: m_trackAuxTasksDecorKeys) {
      track_wdhs.emplace_back(wdhk, ctx);
    }

    // define the required ReadDecorHandle objects
    std::vector<SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char>>> jet_rdhs;
    for (const auto& rdhk: m_readDecorKeys) {
      jet_rdhs.emplace_back(rdhk, ctx);
    }

    // decorate all the tracks with defaults values
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trackContainerKey, ctx);
    ATH_CHECK(tracks.isValid());

    for (const auto track: *tracks) {
      for (auto& wdh: track_wdhs) {
        char value = -1;
        wdh(*track) = value;
      }
    }

    // adding actual aux task results to a subset of tracks
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetContainerKey, ctx);
    ATH_CHECK(jets.isValid());
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>>> trackLinks_rdh(m_trackLinksKey, ctx);
    for (const auto jet: *jets) {
      auto irdh = jet_rdhs.begin();
      auto iwdh = track_wdhs.begin();
      for (; irdh != jet_rdhs.end() && iwdh != track_wdhs.end(); ++irdh, ++iwdh) {
        std::vector<char> values = (*irdh)(*jet);
        auto& trackLinks = trackLinks_rdh(*jet);
        if (values.size() != trackLinks.size()) {
            throw std::logic_error("Track aux task output size " + std::to_string(values.size()) + " doesn't match the size of track list " + std::to_string(trackLinks.size()));
        }
        std::vector<ElementLink<xAOD::TrackParticleContainer>>::const_iterator it = trackLinks.begin();
        std::vector<char>::const_iterator ival = values.begin();
        for (; it != trackLinks.end() && ival != values.end(); ++it, ++ival) {
          (*iwdh)(***it) = *ival;
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode GNNAuxTaskDecoratorAlg::finalize() {
    return StatusCode::SUCCESS;
  }

}
