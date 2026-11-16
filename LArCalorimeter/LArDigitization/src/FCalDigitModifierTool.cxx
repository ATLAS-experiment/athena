/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FCalDigitModifierTool.h"

StatusCode FCalDigitModifierTool::initialize()
{
  ATH_MSG_INFO("FCal signal degradation enabled, ScaleFactor = " << m_scaleFactor.value());
  return StatusCode::SUCCESS;
}

StatusCode FCalDigitModifierTool::modifySamples(const EventContext& /*ctx*/,
                                                const HWIdentifier& /*chId*/,
                                                std::span<double> samples) const
{
  for (double& s : samples) {
    s *= m_scaleFactor.value();
  }
  return StatusCode::SUCCESS;
}
