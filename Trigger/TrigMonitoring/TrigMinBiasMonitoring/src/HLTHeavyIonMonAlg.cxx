/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTHeavyIonMonAlg.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

HLTHeavyIonMonAlg::HLTHeavyIonMonAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode HLTHeavyIonMonAlg::initialize() {
  // for monitorSumEt:
  ATH_CHECK(m_lvl1EnergySumROIKey.initialize());
  ATH_CHECK(m_jFexSumETRoIKey.initialize());
  ATH_CHECK(m_HIEventShapeKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode HLTHeavyIonMonAlg::fillHistograms(const EventContext& context) const {
  ATH_CHECK(monitorSumEt(context));

  return StatusCode::SUCCESS;
}

StatusCode HLTHeavyIonMonAlg::monitorSumEt(const EventContext& context) const {
  // Legacy L1 stuff – left for HLT developer reprocessings
  float lvl1_sumET         = 0.f;
  auto lvl1EnergySumHandle = SG::makeHandle(m_lvl1EnergySumROIKey, context);
  ATH_MSG_DEBUG("LVL1 EnergySum handle: " << lvl1EnergySumHandle.isValid());
  if (lvl1EnergySumHandle.isValid()) { lvl1_sumET = 1e-3 * lvl1EnergySumHandle->energyT(); /* MeV -> GeV */ }

  auto jFexSumETHandle = SG::makeHandle(m_jFexSumETRoIKey, context);
  ATH_MSG_DEBUG("jFeX EnergySum handle: " << jFexSumETHandle.isValid());
  if (not jFexSumETHandle.isValid()) {
    ATH_MSG_DEBUG("Could not retrieve jFexSumETHandle");
    return StatusCode::SUCCESS;
  }

  auto HIEventShapeHandle = SG::makeHandle(m_HIEventShapeKey, context);
  ATH_MSG_DEBUG("HI event shape handle: " << HIEventShapeHandle.isValid());
  if (not HIEventShapeHandle.isValid()) {
    ATH_MSG_DEBUG("Could not retrieve HIEventShapeHandle");
    return StatusCode::SUCCESS;
  }

  // Calculate total and forward TE from jFEX
  float sum_jTE      = 0.f;
  float sum_fwdA_jTE = 0.f;
  float sum_fwdC_jTE = 0.f;

  for (const xAOD::jFexSumETRoI* jFexRoI : *jFexSumETHandle) {
    sum_jTE += jFexRoI->Et_lower() + jFexRoI->Et_upper();
    if (jFexRoI->jFexNumber() == 0) { sum_fwdC_jTE += jFexRoI->Et_upper(); }
    if (jFexRoI->jFexNumber() == 5) { sum_fwdA_jTE += jFexRoI->Et_upper(); }
  }
  sum_jTE *= 0.001; // MeV -> GeV
  sum_fwdA_jTE *= 0.001;
  sum_fwdC_jTE *= 0.001;

  // Calculate FCal ET from HIEventShape
  float totalFCalEtSideA = 0.;
  float totalFCalEtSideC = 0.;
  for (const xAOD::HIEventShape* es : *HIEventShapeHandle) {
    const int layer = es->layer();
    if (layer < 21 or layer > 23) { continue; }

    const float et = es->et();
    if (std::abs(et) < 0.1) { continue; }

    const float eta = 0.5 * (es->etaMin() + es->etaMax());
    if (eta > 0.) {
      totalFCalEtSideA += et;
    } else {
      totalFCalEtSideC += et;
    }
  }
  totalFCalEtSideA *= 0.001; // MeV -> GeV
  totalFCalEtSideC *= 0.001;

  auto sum_L1TE      = Monitored::Scalar("sum_L1TE", lvl1_sumET);
  auto sum_L1jTE     = Monitored::Scalar("sum_L1jTE", sum_jTE);
  auto sum_L1FWDAjTE = Monitored::Scalar("sum_L1FWDAjTE", sum_fwdA_jTE);
  auto sum_L1FWDCjTE = Monitored::Scalar("sum_L1FWDCjTE", sum_fwdC_jTE);
  auto sum_FCalAEt   = Monitored::Scalar("sum_FCalAEt", totalFCalEtSideA);
  auto sum_FCalCEt   = Monitored::Scalar("sum_FCalCEt", totalFCalEtSideC);

  const auto& trigDecTool = getTrigDecisionTool();
  for (const auto& chain : m_triggerListMon) {
    if (not trigDecTool->isPassed(chain, TrigDefs::requireDecision)) { continue; }

    fill(chain + "_sumEt", sum_L1TE, sum_L1jTE, sum_L1FWDAjTE, sum_L1FWDCjTE, sum_FCalAEt, sum_FCalCEt);
  }

  return StatusCode::SUCCESS;
}
