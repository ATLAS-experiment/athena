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

namespace xAOD
{

uint8_t TGCCandData_v1::tcId() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "tcId");
    return acc(*this) & TC_ID_BIT_MASK;
}

uint8_t TGCCandData_v1::passedPtThresholdIndex() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "passedPtThresholdIndex");
    return acc(*this) & PT_THRESHOLD_BIT_MASK;
}

uint8_t TGCCandData_v1::estimatedPtValueIndex() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "estimatedPtValueIndex");
    return acc(*this);
}

float TGCCandData_v1::ptValueGeV() const {
    return 0.5F * static_cast<float>(estimatedPtValueIndex());
}

bool TGCCandData_v1::estimatedPtValueValid() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "estimatedPtValueValid");
    return acc(*this) != 0;
}

bool TGCCandData_v1::side() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "side");
    return (acc(*this) & 0x1) != 0;
}

uint8_t TGCCandData_v1::endcapFlag() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "endcapFlag");
    return acc(*this) & 0x1;
}

uint8_t TGCCandData_v1::sector() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "sector");
    return acc(*this) & 0x3f;
}

bool TGCCandData_v1::hasInnerCoincidence() const {
    return (coinType() & COINTYPE_INNER_COINCIDENCE_BIT) != 0;
}

bool TGCCandData_v1::goodMagneticField() const {
    return (coinType() & COINTYPE_GOOD_MAGNETIC_FIELD_BIT) != 0;
}

float TGCCandData_v1::deltaPhi() const {
    return decodeSigned(deltaPhiWord(), s_dphiRange, DPHI_BIT_MASK);
}

float TGCCandData_v1::deltaTheta() const {
    return decodeSigned(deltaThetaWord(), s_dthetaRange, DTHETA_BIT_MASK);
}

uint8_t TGCCandData_v1::deltaPhiWord() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "deltaPhi");
    return acc(*this);
}

uint8_t TGCCandData_v1::deltaThetaWord() const {
    static const SG::ConstAccessor<uint8_t> acc(preFixStr + "deltaTheta");
    return acc(*this);
}

uint32_t TGCCandData_v1::nswSegment() const {
    static const SG::ConstAccessor<uint32_t> acc(preFixStr + "nswSegment");
    return acc(*this) & NSW_BIT_MASK;
}

void TGCCandData_v1::setTcId(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "tcId");
    acc(*this) = value & TC_ID_BIT_MASK;
}

void TGCCandData_v1::setPassedPtThresholdIndex(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "passedPtThresholdIndex");
    acc(*this) = std::min(value, PT_THRESHOLD_BIT_MASK);
}

void TGCCandData_v1::setEstimatedPtValueIndex(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "estimatedPtValueIndex");
    acc(*this) = value;
}

void TGCCandData_v1::setEstimatedPtValueValid(bool value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "estimatedPtValueValid");
    acc(*this) = value ? 1 : 0;
}

void TGCCandData_v1::setSide(bool value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "side");
    acc(*this) = value ? 1 : 0;
}

void TGCCandData_v1::setEndcapFlag(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "endcapFlag");
    acc(*this) = value & 0x1;
}

void TGCCandData_v1::setSector(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "sector");
    acc(*this) = value & 0x3f;
}

void TGCCandData_v1::setHasInnerCoincidence(bool value) {
    setCoinType(value ? (coinType() | COINTYPE_INNER_COINCIDENCE_BIT)
                      : (coinType() & static_cast<uint8_t>(~COINTYPE_INNER_COINCIDENCE_BIT)));
}

void TGCCandData_v1::setGoodMagneticField(bool value) {
    setCoinType(value ? (coinType() | COINTYPE_GOOD_MAGNETIC_FIELD_BIT)
                      : (coinType() & static_cast<uint8_t>(~COINTYPE_GOOD_MAGNETIC_FIELD_BIT)));
}

void TGCCandData_v1::setDeltaPhi(float dphi) {
    setDeltaPhiWord(encodeSigned(dphi, s_dphiRange, DPHI_BIT_MASK));
}

void TGCCandData_v1::setDeltaTheta(float dtheta) {
    setDeltaThetaWord(encodeSigned(dtheta, s_dthetaRange, DTHETA_BIT_MASK));
}

void TGCCandData_v1::setDeltaPhiWord(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "deltaPhi");
    acc(*this) = value & static_cast<uint8_t>((DPHI_BIT_MASK << 1) | 0x1);
}

void TGCCandData_v1::setDeltaThetaWord(uint8_t value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "deltaTheta");
    acc(*this) = value & static_cast<uint8_t>((DTHETA_BIT_MASK << 1) | 0x1);
}

void TGCCandData_v1::setNswSegment(uint32_t nswout) {
    static const SG::Accessor<uint32_t> acc(preFixStr + "nswSegment");
    acc(*this) = nswout & NSW_BIT_MASK;
}

} // namespace xAOD
