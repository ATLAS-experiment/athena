/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RatesEmulationExample.h"

#include <xAODEgamma/ElectronContainer.h>
#include <xAODJet/JetContainer.h>

RatesEmulationExample::RatesEmulationExample( const std::string& name, ISvcLocator* pSvcLocator ) : RatesAnalysisAlg(name, pSvcLocator) {
}

RatesEmulationExample::~RatesEmulationExample() {
}

StatusCode RatesEmulationExample::initialize_extra_content() {
  // Read Handle Key
  ATH_CHECK( m_electron_RHKey.initialize());
  ATH_CHECK( m_jet_RHKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode RatesEmulationExample::ratesInitialize() {
  ATH_MSG_DEBUG("In ratesInitialize()");
  
  // Here we assume a full-ring, other functions are available to change this assumption.
  // @see setTargetLumiMu(const double lumi, const double mu);
  // @see setTargetLumiBunches(const double lumi, const int32_t bunches);
  // @see setTargetMuBunches(const double mu, const int32_t bunches);
  setTargetLumi( m_lumi );

  // Define triggers to emulate
  // TDT can be used instead by ATH_CHECK(addAllExisting());

  // name, prescale, expressPrescale, seedName, seedPrescale, groups
  ATH_CHECK(newTrigger("OFF_e10", 1, -1, "", 1, std::set<std::string>{"RATE_SingleElectron"}));
  ATH_CHECK(newTrigger("OFF_j40c_AntiKt4EMTopo", 1, -1, "", 1, std::set<std::string>{"RATE_SingleJet"}));

  // name, thresholdMin, thresholdMax, bins (optional)
  ATH_CHECK(newScanTrigger("OFF_eX", 20, 40));

  // name, thresholdMin, thresholdMax
  ATH_CHECK(newScanTrigger("OFF_jXa_AntiKt4EMTopo", 5, 4000, 799));
  ATH_CHECK(newScanTrigger("OFF_jXc_AntiKt4EMTopo", 5, 4000, 799));
  ATH_CHECK(newScanTrigger("OFF_3jXc_AntiKt4EMTopo", 5, 1000, 499));

  return StatusCode::SUCCESS;
}

StatusCode RatesEmulationExample::ratesExecute() {

  SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electron_RHKey);
  ATH_CHECK( electrons.isValid() );
  std::set<double> electronpTs;
  for (const auto e : *electrons) electronpTs.insert(e->pt()/1000.);
  if (electronpTs.size() >= 1 && *electronpTs.rbegin() >= 10.) ATH_CHECK(setTriggerDesicison("OFF_e10", true));
  if (electronpTs.size() >= 1) ATH_CHECK(setTriggerDesicison("OFF_eX", *electronpTs.rbegin() ));


  SG::ReadHandle<xAOD::JetContainer> Jets(m_jet_RHKey);
  ATH_CHECK( Jets.isValid() );

  std::set<double> jetpTs;
  for (const auto j : *Jets) jetpTs.insert(j->pt()/1000.);
  if (jetpTs.size() >= 1) ATH_CHECK(setTriggerDesicison("OFF_jXa_AntiKt4EMTopo", *jetpTs.rbegin() ));

  std::set<double> jetcpTs;
  for (const auto jc : *Jets) {
    if (std::abs(jc->eta()) < 2.4) jetcpTs.insert(jc->pt()/1000.);
  }
  if (jetcpTs.size() >= 1 && *jetcpTs.rbegin() >= 40.) ATH_CHECK(setTriggerDesicison("OFF_j40c_AntiKt4EMTopo", true));
  if (jetcpTs.size() >= 1) ATH_CHECK(setTriggerDesicison("OFF_jXc_AntiKt4EMTopo", *jetcpTs.rbegin() ));
  if (jetcpTs.size() >= 3) ATH_CHECK(setTriggerDesicison("OFF_3jXc_AntiKt4EMTopo", *jetcpTs.rbegin()+2 ));

  return StatusCode::SUCCESS;
}

StatusCode RatesEmulationExample::ratesFinalize() {
  ATH_MSG_DEBUG("In ratesFinalize()");
  return StatusCode::SUCCESS;
}

