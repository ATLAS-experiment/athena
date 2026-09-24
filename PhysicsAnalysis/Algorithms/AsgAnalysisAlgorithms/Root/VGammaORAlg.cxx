/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Lucas Cremer

#include "AsgAnalysisAlgorithms/VGammaORAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <EventBookkeeperTools/FilterReporter.h>

namespace CP {

  StatusCode VGammaORAlg::initialize() {

    ANA_CHECK(m_vgammaORTool.retrieve());
    ANA_CHECK(m_filterParams.initialize());
    ANA_CHECK(m_eventInfoKey.initialize());
    ANA_CHECK(m_inOverlapKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode VGammaORAlg::execute(const EventContext& ctx) const {

    FilterReporter filter(m_filterParams, m_noFilter.value(), ctx);

    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfoKey, ctx);
    ANA_CHECK(evtInfo.isValid());

    bool in_vgamma_overlap;
    ANA_CHECK(m_vgammaORTool->inOverlap(in_vgamma_overlap));

    SG::WriteDecorHandle<xAOD::EventInfo, bool> dec(m_inOverlapKey, ctx);
    dec(*evtInfo) = in_vgamma_overlap;

    if (!m_noFilter.value()) {
      if (m_keepOverlap)
        filter.setPassed(  in_vgamma_overlap );
      else
        filter.setPassed( !in_vgamma_overlap );
    }

    return StatusCode::SUCCESS;
  }

  StatusCode VGammaORAlg::finalize() {

    ANA_MSG_INFO(m_filterParams.summary());
    return StatusCode::SUCCESS;
  }
} // namespace
