/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/SaveFilterAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace CP {

  SaveFilterAlg::SaveFilterAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
    {}

  StatusCode SaveFilterAlg::initialize() {
    ANA_CHECK(m_filterParams.initialize(m_systematicsList));

    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_inputselection.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_outputselection.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode SaveFilterAlg::execute() {
    // the event-level filter
    CP::SysFilterReporterCombiner filterCombiner(m_filterParams, m_noFilter.value());

    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // the per-systematic filter
      CP::SysFilterReporter filter(filterCombiner, sys);

      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo {nullptr};
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_outputselection.setBool(*evtInfo, 0, sys);
      m_decoration.set(*evtInfo, 0, sys);

      // check the selections
      bool passSelection = m_inputselection.getBool(*evtInfo, sys);
      if (passSelection) {
	filter.setPassed(true);
	m_outputselection.setBool(*evtInfo, passSelection, sys);
	m_decoration.set(*evtInfo, passSelection, sys);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode SaveFilterAlg::finalize() {
    ANA_CHECK(m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }

} // namespace CP
