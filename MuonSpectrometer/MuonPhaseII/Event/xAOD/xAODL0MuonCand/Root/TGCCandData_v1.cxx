/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/TGCCandData.h"

#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD
{
  
float TGCCandData_v1::deltaPhi() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "deltaPhi");
    uint8_t dphiBin = acc(*this);
    float dphi = static_cast<float>(dphiBin & DPHI_BIT_MASK) / static_cast<float>(DPHI_BIT_MASK+1) * s_dphiRange;
    return (dphiBin & (DPHI_BIT_MASK + 1)) ? dphi : -1. * dphi;
}

float TGCCandData_v1::deltaTheta() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "deltaTheta");
    uint8_t dthetaBin = acc(*this);
    float dtheta = static_cast<float>(dthetaBin & DTHETA_BIT_MASK) / static_cast<float>(DTHETA_BIT_MASK+1) * s_dthetaRange;
    return (dthetaBin & (DTHETA_BIT_MASK + 1)) ? dtheta : -1. * dtheta;
}

uint32_t TGCCandData_v1::nswSegment() const {
    static const SG::ConstAccessor<uint32_t> acc(preFixStr + "nswSegment");
    return acc(*this);
}

void TGCCandData_v1::setDeltaPhi(float dphi) {
  uint8_t deltaPhiBin = std::min(static_cast<uint8_t>(std::abs(dphi) / s_dphiRange * static_cast<float>(DPHI_BIT_MASK+1)),
                        static_cast<uint8_t>(DPHI_BIT_MASK));
  if (dphi > 0.) deltaPhiBin += DPHI_BIT_MASK + 1;
  static const SG::Accessor<uint8_t> acc(preFixStr + "deltaPhi");
  acc(*this) = deltaPhiBin;
}

void TGCCandData_v1::setDeltaTheta(float dtheta) {
  uint8_t deltaThetaBin = std::min(static_cast<uint8_t>(std::abs(dtheta) / s_dthetaRange * static_cast<float>(DTHETA_BIT_MASK+1)),
                          static_cast<uint8_t>(DTHETA_BIT_MASK));
  if (dtheta > 0.)  deltaThetaBin += DTHETA_BIT_MASK + 1;
  static const SG::Accessor<uint8_t> acc(preFixStr + "deltaTheta");
  acc(*this) = deltaThetaBin;
}

void TGCCandData_v1::setNswSegment(uint32_t nswout) {
  static const SG::Accessor<uint32_t> acc(preFixStr + "nswSegment");
  acc(*this) = nswout;
}

} // namespace xAOD

