/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/TgcDigitCrosstalkData.h"

TgcDigitCrosstalkData::TgcDigitCrosstalkData()
{}

void TgcDigitCrosstalkData::setStripProbability(const uint16_t layer_id, const std::array<float, N_PROB> prob)
{
  m_stripProb.emplace(layer_id, prob);
}

float TgcDigitCrosstalkData::getStripProbability(const uint16_t layer_id, const unsigned int index_prob) const
{
  if (index_prob >= N_PROB) return 0.;
  auto pLayer = m_stripProb.find(layer_id);
  if (pLayer == m_stripProb.end()) return 0.;
  return pLayer->second[index_prob];
}

void TgcDigitCrosstalkData::setWireProbability(const uint16_t layer_id, const std::array<float, N_PROB> prob)
{
  m_wireProb.emplace(layer_id, prob);
}

float TgcDigitCrosstalkData::getWireProbability(const uint16_t layer_id, const unsigned int index_prob) const
{
  if (index_prob >= N_PROB) return 0.;
  auto pLayer = m_wireProb.find(layer_id);
  if (pLayer == m_wireProb.end()) return 0.;
  return pLayer->second[index_prob];
}
