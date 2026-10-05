/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/L1TGCCandData.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include <cmath>
#include <algorithm>

namespace {

   // The maximum magnitude code maps to the documented compact-word range.
   constexpr uint8_t encodeSigned(float value, float range, uint8_t magnitudeMask) {
      const float absValue = std::min(std::abs(value), range);
      uint8_t word = static_cast<uint8_t>(std::lround(absValue / range *
                                                      static_cast<float>(magnitudeMask)));
      word &= magnitudeMask;
      if (value < 0.F) word |= static_cast<uint8_t>(magnitudeMask + 1);
      return word;
   }

   constexpr float decodeSigned(uint8_t word, float range, uint8_t magnitudeMask) {
      const float magnitude = static_cast<float>(word & magnitudeMask) /
                              static_cast<float>(magnitudeMask) * range;
      return (word & static_cast<uint8_t>(magnitudeMask + 1)) ? -magnitude : magnitude;
   }
}

namespace xAOD {

float L1TGCCandData_v1::ptValueGeV() const {
    return ptResolution() * static_cast<float>(l1Pt());
}

bool L1TGCCandData_v1::hasInnerCoincidence() const {
    return (coinType() & COINTYPE_INNER_COINCIDENCE_BIT) != 0;
}

bool L1TGCCandData_v1::goodMagneticField() const {
    return (coinType() & COINTYPE_GOOD_MAGNETIC_FIELD_BIT) != 0;
}

float L1TGCCandData_v1::deltaPhi() const {
    return decodeSigned(l1DeltaPhiWord(), s_dphiRange, DPHI_BIT_MASK);
}

float L1TGCCandData_v1::deltaTheta() const {
    return decodeSigned(l1DeltaThetaWord(), s_dthetaRange, DTHETA_BIT_MASK);
}

uint8_t L1TGCCandData_v1::l1DeltaPhiWord() const {
    static const SG::ConstAccessor<uint8_t> acc("l1DeltaPhiWord");
    return acc(*this);
}

uint8_t L1TGCCandData_v1::l1DeltaThetaWord() const {
    static const SG::ConstAccessor<uint8_t> acc("l1DeltaThetaWord");
    return acc(*this);
}

uint32_t L1TGCCandData_v1::l1NswSegment() const {
    static const SG::ConstAccessor<uint32_t> acc("l1NswSegment");
    return acc(*this) & NSW_BIT_MASK;
}

void L1TGCCandData_v1::setHasInnerCoincidence(bool value) {
    setCoinType(value ? (coinType() | COINTYPE_INNER_COINCIDENCE_BIT)
                      : (coinType() & static_cast<uint8_t>(~COINTYPE_INNER_COINCIDENCE_BIT)));
}

void L1TGCCandData_v1::setGoodMagneticField(bool value) {
    setCoinType(value ? (coinType() | COINTYPE_GOOD_MAGNETIC_FIELD_BIT)
                      : (coinType() & static_cast<uint8_t>(~COINTYPE_GOOD_MAGNETIC_FIELD_BIT)));
}

void L1TGCCandData_v1::setDeltaPhi(float dphi) {
    setL1DeltaPhiWord(encodeSigned(dphi, s_dphiRange, DPHI_BIT_MASK));
}

void L1TGCCandData_v1::setDeltaTheta(float dtheta) {
    setL1DeltaThetaWord(encodeSigned(dtheta, s_dthetaRange, DTHETA_BIT_MASK));
}

void L1TGCCandData_v1::setL1DeltaPhiWord(uint8_t value) {
    static const SG::Accessor<uint8_t> acc("l1DeltaPhiWord");
    acc(*this) = value & static_cast<uint8_t>((DPHI_BIT_MASK << 1) | 0x1);
}

void L1TGCCandData_v1::setL1DeltaThetaWord(uint8_t value) {
    static const SG::Accessor<uint8_t> acc("l1DeltaThetaWord");
    acc(*this) = value & static_cast<uint8_t>((DTHETA_BIT_MASK << 1) | 0x1);
}

void L1TGCCandData_v1::setL1NswSegment(uint32_t nswout) {
    static const SG::Accessor<uint32_t> acc("l1NswSegment");
    acc(*this) = nswout & NSW_BIT_MASK;
}

} // namespace xAOD
