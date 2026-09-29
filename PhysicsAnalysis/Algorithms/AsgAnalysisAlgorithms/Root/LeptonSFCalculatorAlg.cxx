/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "AsgAnalysisAlgorithms/LeptonSFCalculatorAlg.h"

namespace CP {

  StatusCode LeptonSFCalculatorAlg::initialize() {

    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_photonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_tausHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_photonSelection.initialize(m_systematicsList, m_photonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_tauSelection.initialize(m_systematicsList, m_tausHandle, SG::AllowEmpty));

    ANA_CHECK(m_electronSFs.initialize(m_systematicsList, m_electronsHandle));
    ANA_CHECK(m_muonSFs.initialize(m_systematicsList, m_muonsHandle));
    ANA_CHECK(m_photonSFs.initialize(m_systematicsList, m_photonsHandle));
    ANA_CHECK(m_tauSFs.initialize(m_systematicsList, m_tausHandle));

    ANA_CHECK(m_event_leptonSF.initialize(m_systematicsList, m_eventInfoHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode LeptonSFCalculatorAlg::execute(const EventContext& ctx) {
    for (const auto& syst : m_systematicsList.systematicsVector()) {
      const xAOD::EventInfo *evtInfo {nullptr};
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, syst, ctx));

      double leptonSF {1.};
      auto accumulateSFs = [&]<typename T>(CP::SysReadHandle<T>& handle,
                                           CP::SysReadSelectionHandle& selection,
                                           CP::SysReadDecorHandleArray<float>& sfs) -> StatusCode {
        if (!handle) return StatusCode::SUCCESS;
        const T *particles {nullptr};
        ANA_CHECK(handle.retrieve(particles, syst, ctx));
        for (const auto *particle : *particles) {
          if (selection.getBool(*particle, syst)) {
            for (size_t i{}; i < sfs.size(); i++) {
              leptonSF *= sfs.at(i).get(*particle, syst);
            }
          }
        }
        return StatusCode::SUCCESS;
      };
      ANA_CHECK(accumulateSFs(m_electronsHandle, m_electronSelection, m_electronSFs));
      ANA_CHECK(accumulateSFs(m_muonsHandle, m_muonSelection, m_muonSFs));
      ANA_CHECK(accumulateSFs(m_photonsHandle, m_photonSelection, m_photonSFs));
      ANA_CHECK(accumulateSFs(m_tausHandle, m_tauSelection, m_tauSFs));

      m_event_leptonSF.set(*evtInfo, leptonSF, syst);
    }
    return StatusCode::SUCCESS;
  }

} // namespace
