/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/SumNLeptonPtSelectorAlg.h"

namespace CP {

  SumNLeptonPtSelectorAlg::SumNLeptonPtSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode SumNLeptonPtSelectorAlg::initialize() {
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_tausHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_tauSelection.initialize(m_systematicsList, m_tausHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode SumNLeptonPtSelectorAlg::execute() {
    // accessors
    static const SG::AuxElement::ConstAccessor<float> acc_pt_dressed("pt_dressed");

    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the electron container
      const xAOD::IParticleContainer *electrons = nullptr;
      if (m_electronsHandle)
        ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      // retrieve the muon container
      const xAOD::IParticleContainer *muons = nullptr;
      if (m_muonsHandle)
        ANA_CHECK(m_muonsHandle.retrieve(muons, sys));
      // retrieve the tau container
      const xAOD::IParticleContainer *taus = nullptr;
      if (m_tausHandle)
        ANA_CHECK(m_tausHandle.retrieve(taus, sys));

      // apply the requested selection
      int count = 0;
      if (m_electronsHandle) {
        for (const xAOD::IParticle *el : *electrons) {
          if (!m_electronSelection || m_electronSelection.getBool(*el, sys)) {
            if (m_useDressedProperties) {
              if (acc_pt_dressed(*el) > m_elptmin) count++;
            } else {
              if (el->pt() > m_elptmin) count++;
            }
          }
        }
      }
      if (m_muonsHandle) {
        for (const xAOD::IParticle *mu : *muons) {
          if (!m_muonSelection || m_muonSelection.getBool(*mu, sys)) {
              if (m_useDressedProperties) {
                if (acc_pt_dressed(*mu) > m_muptmin) count++;
              } else {
                if (mu->pt() > m_muptmin) count++;
            }
          }
        }
      }
      if (m_tausHandle) {
        for (const xAOD::IParticle *tau : *taus) {
          if (!m_tauSelection || m_tauSelection.getBool(*tau, sys)) {
            // for taus dressed properties do not make a big difference
            if (tau->pt() > m_tauptmin) count++;
          }
        }
      }

      // calculate decision
      bool decision = SignEnum::checkValue(m_count.value(), m_signEnum, count);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
