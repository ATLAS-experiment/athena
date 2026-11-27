/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FCalMuCorrTool.h"

#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

StatusCode FCalMuCorrTool::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_caloID, "CaloCell_ID"));
  ATH_CHECK(m_eventInfoKey.initialize(m_muOverride.value() <= 0.));

  ATH_CHECK(m_degKey.initialize());

  if (m_muOverride.value() > 0.) {
    ATH_MSG_INFO("FCal1 mu correction enabled: " << "table " << m_degKey.key() << ", MuOverride = " << m_muOverride.value());
  } else {
    ATH_MSG_INFO("FCal1 mu correction enabled: " << "table " << m_degKey.key() << ", mu from EventInfo");
  }
  return StatusCode::SUCCESS;
}

StatusCode FCalMuCorrTool::process(CaloCellContainer* cellCollection, const EventContext& ctx) const
{
  if (!cellCollection) {
    ATH_MSG_ERROR("Cell Correction tool receives invalid cell Collection");
    return StatusCode::FAILURE;
  }

  double mu = m_muOverride.value();
  if (mu <= 0.) {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());
    mu = eventInfo->averageInteractionsPerCrossing();
  }

  SG::ReadCondHandle<LArFcalDegFactors> degradation(m_degKey, ctx);
  ATH_CHECK(degradation.isValid());

  for (CaloCell* theCell : *cellCollection) {
    const Identifier id = theCell->ID();
    // only FCal1 is affected
    if (m_caloID->calo_sample(id) != CaloSampling::FCAL0) continue;
    const int channel = m_caloID->eta(id);
    if (channel < 0 || static_cast<size_t>(channel) >= degradation->nChannels()) continue;
    const double factor = degradation->factor(channel, mu);
    theCell->scaleEnergy(1. / factor);
  }
  return StatusCode::SUCCESS;
}
