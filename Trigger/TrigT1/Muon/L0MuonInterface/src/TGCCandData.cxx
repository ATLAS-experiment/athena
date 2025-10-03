/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "L0MuonInterface/TGCCandData.h"

namespace L0Muon {

uint8_t TGCCandData::coinType() const {
  return m_coinType;
}

float TGCCandData::deltaPhi() const {
  float dphi = static_cast<float>(m_deltaPhi & DPHI_BIT_MASK) / static_cast<float>(DPHI_BIT_MASK+1) * s_dphiRange;
  return (m_deltaPhi & (DPHI_BIT_MASK + 1)) ? dphi : -1. * dphi;
}

float TGCCandData::deltaTheta() const {
  float dtheta = static_cast<float>(m_deltaTheta & DTHETA_BIT_MASK) / static_cast<float>(DTHETA_BIT_MASK+1) * s_dthetaRange;
  return (m_deltaTheta & (DTHETA_BIT_MASK + 1)) ? dtheta : -1. * dtheta;
}

uint32_t TGCCandData::nswSegment() const {
  return m_nswSegment;
}


void TGCCandData::setCoinType(uint8_t cointype) {
  m_coinType = cointype & COINTYPE_BIT_MASK;
}

void TGCCandData::setDeltaPhi(float dphi) {
  m_deltaPhi = std::min(static_cast<uint8_t>(std::abs(dphi) / s_dphiRange * static_cast<float>(DPHI_BIT_MASK+1)),
                        static_cast<uint8_t>(DPHI_BIT_MASK));
  if (dphi > 0.) m_deltaPhi += DPHI_BIT_MASK + 1;
}

void TGCCandData::setDeltaTheta(float dtheta) {
  m_deltaTheta = std::min(static_cast<uint8_t>(std::abs(dtheta) / s_dthetaRange * static_cast<float>(DTHETA_BIT_MASK+1)),
                          static_cast<uint8_t>(DTHETA_BIT_MASK));
  if (dtheta > 0.)  m_deltaTheta += DTHETA_BIT_MASK + 1;
}

void TGCCandData::setNswSegment(uint32_t nswout) {
  m_nswSegment = nswout;
}

}  // end of L0Muon namespace
