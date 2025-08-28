/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/JetNGhostSelectorAlg.h"

namespace CP {

  JetNGhostSelectorAlg::JetNGhostSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode JetNGhostSelectorAlg::initialize() {
    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    m_ghostAcc = std::make_unique<SG::AuxElement::ConstAccessor<int>> (m_ghost.value());
    if ( !m_veto.value().empty() ) {
      m_vetoAcc= std::make_unique<SG::AuxElement::ConstAccessor<int>> (m_veto.value());
      m_doVeto = true;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetNGhostSelectorAlg::execute() {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the jet container
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys));

      // apply and calculate the decision
      int count = 0;
      for (const xAOD::Jet *jet : *jets){
        if (!m_jetSelection || m_jetSelection.getBool(*jet, sys)){
          if (jet->pt() >= m_ptmin){
            if (!m_ghostAcc->isAvailable(*jet)) {
              ANA_MSG_ERROR ("Ghost decoration " << m_ghost.value() << " is not available on this jet!");
              return StatusCode::FAILURE;
            }
            if (m_doVeto) {
              if (!m_vetoAcc->isAvailable(*jet)) {
                ANA_MSG_ERROR ("Ghost decoration " << m_veto.value() << " is not available on this jet!");
              return StatusCode::FAILURE;
              }
              if ( (*m_vetoAcc)(*jet) > 0 ) continue;
            }
            if ( (*m_ghostAcc)(*jet) > 0 ) count++;
          }
        }
      }

      bool decision = SignEnum::checkValue(m_count.value(), m_signEnum, count);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
