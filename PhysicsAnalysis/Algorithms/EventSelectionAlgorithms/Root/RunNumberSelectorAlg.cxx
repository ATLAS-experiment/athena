/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/RunNumberSelectorAlg.h"

namespace CP {

  StatusCode RunNumberSelectorAlg::initialize() {
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode RunNumberSelectorAlg::execute() {

    static const SG::Accessor<unsigned int> acc_random("RandomRunNumber");

    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the runNumber
      unsigned int runNumber = 0;
      if (m_useRandomRunNumber) runNumber = acc_random(*evtInfo);
      else runNumber = evtInfo->runNumber();
      
      // calculate decision
      bool decision = SignEnum::checkValue(m_runNumberRef.value(), m_signEnum, runNumber);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
