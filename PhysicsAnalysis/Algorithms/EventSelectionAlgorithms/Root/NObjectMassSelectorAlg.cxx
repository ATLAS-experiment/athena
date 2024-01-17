/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/NObjectMassSelectorAlg.h"

namespace CP {

  NObjectMassSelectorAlg::NObjectMassSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode NObjectMassSelectorAlg::initialize() {
    ANA_CHECK(m_objectsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_objectSelection.initialize(m_systematicsList, m_objectsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode NObjectMassSelectorAlg::execute() {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the object container
      const xAOD::IParticleContainer *objects = nullptr;
      ANA_CHECK(m_objectsHandle.retrieve(objects, sys));

      // apply and calculate the decision
      int count = 0;
      for (const xAOD::IParticle *obj : *objects){
        if (!m_objectSelection || m_objectSelection.getBool(*obj, sys)){
          if (obj->m() > m_minmass){
            count++; 
          }
        }
      }

      bool decision = SignEnum::checkValue(m_count.value(), m_signEnum, count);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
