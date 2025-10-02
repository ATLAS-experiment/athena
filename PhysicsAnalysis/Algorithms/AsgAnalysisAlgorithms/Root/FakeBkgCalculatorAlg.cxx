/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nello Bruscino

#include "AsgAnalysisAlgorithms/FakeBkgCalculatorAlg.h"
#include "AthContainers/ConstDataVector.h"

namespace CP {

  StatusCode FakeBkgCalculatorAlg::initialize() {

    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle));
    ANA_CHECK(m_electronSelectionTarget.initialize(m_systematicsList, m_electronsHandle));

    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle));
    ANA_CHECK(m_muonSelectionTarget.initialize(m_systematicsList, m_muonsHandle));

    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));

    ANA_CHECK(m_fakeTool.retrieve());
    ANA_CHECK(m_fakeToolOutput.initialize(m_systematicsList, m_eventInfoHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode FakeBkgCalculatorAlg::execute() {

    static const SG::Decorator<char> dec_lepton_tight("TightForFakeBkgCalculation");
    static const SG::Accessor<char> flagAcc("TightForFakeBkgCalculation");

    ANA_MSG_DEBUG(" ----> FakeBkgCalculatorAlg::execute()");
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      const xAOD::EventInfo *evtInfo = nullptr;
      const xAOD::ElectronContainer *electrons = nullptr;
      const xAOD::MuonContainer *muons = nullptr;

      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      m_fakeToolOutput.set(*evtInfo, -1, sys); // default value
      if ( m_preselection && !m_preselection.getBool(*evtInfo, sys)) continue;

      ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      ANA_CHECK(m_muonsHandle.retrieve(muons, sys));

      ConstDataVector<xAOD::IParticleContainer> leptons(SG::VIEW_ELEMENTS);
      for (const xAOD::Electron *t : *electrons) {
	if( m_electronSelection && !m_electronSelection.getBool(*t, sys)) continue;
	leptons.push_back(t);
	dec_lepton_tight(*leptons.back()) = m_electronSelectionTarget.getBool(*t, sys);
      }
      for (const xAOD::Muon *t : *muons) {
	if( m_muonSelection && !m_muonSelection.getBool(*t, sys)) continue;
	leptons.push_back(t);
	dec_lepton_tight(*leptons.back()) = m_muonSelectionTarget.getBool(*t, sys);
      }

      for (const xAOD::IParticle *t : leptons)
	ANA_MSG_DEBUG("lepton type = "<<t->type()<<", pt = "<<t->pt()<<" , eta = "<<t->eta()<<", tight = "<<bool(flagAcc(*t)));

      ANA_CHECK(m_fakeTool->addEvent(leptons));
      float asmWgt = 0.;
      ANA_CHECK(m_fakeTool->applySystematicVariation({})); // the nominal MM weight is computed
      ANA_CHECK(m_fakeTool->getEventWeight(asmWgt, m_definition, m_process));
      ANA_MSG_DEBUG(" ----> asmWgt = "<<asmWgt);

      m_fakeToolOutput.set(*evtInfo, asmWgt, sys);
    }

    return StatusCode::SUCCESS;
  }

} // namespace
