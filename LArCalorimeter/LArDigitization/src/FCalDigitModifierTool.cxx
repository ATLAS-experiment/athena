/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FCalDigitModifierTool.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

StatusCode FCalDigitModifierTool::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_caloID, "CaloCell_ID"));
  ATH_CHECK(m_eventInfoKey.initialize(m_muOverride.value() <= 0.));

  ATH_CHECK(m_degKey.initialize());

  if (m_muOverride.value() > 0.) {
    ATH_MSG_INFO("FCal1 signal degradation enabled: " << "table " << m_degKey.key() << ", MuOverride = " << m_muOverride.value());
  } else {
    ATH_MSG_INFO("FCal1 signal degradation enabled: " << "table " << m_degKey.key() << ", mu from EventInfo");
  }
  return StatusCode::SUCCESS;
}

StatusCode FCalDigitModifierTool::prepareEvent(const EventContext& ctx, EventData& data) const
{
  SG::ReadCondHandle<LArFcalDegFactors> degradation(m_degKey, ctx);
  ATH_CHECK(degradation.isValid());
  data.degradation = *degradation;

  data.mu = m_muOverride.value();
  if (data.mu <= 0.) {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());
    data.mu = eventInfo->averageInteractionsPerCrossing();
  }
  return StatusCode::SUCCESS;
}

void FCalDigitModifierTool::modifySamples(const EventData& data,
                                          const Identifier& cellId,
                                          std::span<double> samples) const
{
  if (!data.degradation) return;
  // only FCal1 is affected
  if (m_caloID->calo_sample(cellId) != CaloSampling::FCAL0) return;
  const int channel = m_caloID->eta(cellId);
  if (channel < 0 || static_cast<size_t>(channel) >= data.degradation->nChannels()) return;

  const double factor = data.degradation->factor(channel, data.mu);
  for (double& s : samples) {
    s *= factor;
  }
}
