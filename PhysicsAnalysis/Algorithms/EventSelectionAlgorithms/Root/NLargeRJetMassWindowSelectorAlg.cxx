/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/NLargeRJetMassWindowSelectorAlg.h"

namespace CP {

  NLargeRJetMassWindowSelectorAlg::NLargeRJetMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode NLargeRJetMassWindowSelectorAlg::initialize() {
    ANA_CHECK(m_ljetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_ljetSelection.initialize(m_systematicsList, m_ljetsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode NLargeRJetMassWindowSelectorAlg::execute() {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the large-R jet container
      const xAOD::IParticleContainer *ljets = nullptr;
      ANA_CHECK(m_ljetsHandle.retrieve(ljets, sys));

      // apply selection and calculate the jet-wise decision
      int count = 0;
      for (const xAOD::IParticle *lj : *ljets) {
        if (!m_ljetSelection || m_ljetSelection.getBool(*lj, sys)) {
	  double mass = lj->m();
	  bool in_range = ( mass > m_mlower && mass < m_mupper );
	  bool pass = m_veto ? (!in_range) : in_range;
	  if (pass) count++;
	}
      }

      // calculate event-wise decision
      bool decision = SignEnum::checkValue(m_count.value(), m_signEnum, count);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
