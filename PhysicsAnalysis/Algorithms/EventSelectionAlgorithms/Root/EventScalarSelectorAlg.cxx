/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/EventScalarSelectorAlg.h"

namespace CP {

  EventScalarSelectorAlg::EventScalarSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode EventScalarSelectorAlg::initialize() {
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_floatVariable.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_intVariable.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_doubleVariable.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    // exactly one typed handle must be configured
    const int nConfigured = (m_floatVariable ? 1 : 0)
                          + (m_intVariable ? 1 : 0)
                          + (m_doubleVariable ? 1 : 0);
    if (nConfigured != 1) {
      ANA_MSG_ERROR("EVENTVAR: exactly one of floatVariable / intVariable / "
                    "doubleVariable must be configured (got " << nConfigured << ")");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode EventScalarSelectorAlg::execute(const EventContext& ctx) {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys, ctx));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // read the configured scalar, promoted to double for the comparison
      double value = 0.;
      if (m_floatVariable)       value = m_floatVariable.get(*evtInfo, sys);
      else if (m_intVariable)    value = m_intVariable.get(*evtInfo, sys);
      else if (m_doubleVariable) value = m_doubleVariable.get(*evtInfo, sys);

      // calculate decision
      bool decision = SignEnum::checkValue(static_cast<double>(m_refValue.value()), m_signEnum, value);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }

} // namespace CP
