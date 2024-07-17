/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Lucas Cremer

#include "AsgAnalysisAlgorithms/VGammaORAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace CP {

  StatusCode VGammaORAlg::initialize() {

    ANA_CHECK(m_filterParams.initialize(m_systematicsList));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_inOverlapHandle.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode VGammaORAlg::execute() {

    // the event-level filter
    CP::SysFilterReporterCombiner filterCombiner(m_filterParams, m_noFilter.value());

    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // the per-systematic filter
      CP::SysFilterReporter filter(filterCombiner, sys);

      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      bool in_vgamma_overlap;
      ANA_CHECK(m_vgammaORTool->inOverlap(in_vgamma_overlap));
      m_inOverlapHandle.set(*evtInfo, in_vgamma_overlap, sys);

      if (m_keepOverlap)
	filter.setPassed(  in_vgamma_overlap );
      else
	filter.setPassed( !in_vgamma_overlap );
    }

    return StatusCode::SUCCESS;
  }

  StatusCode VGammaORAlg::finalize() {

    ANA_CHECK(m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }
} // namespace
