/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/versions/ICandData_v1.h"

#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include <cmath>
#include <algorithm>
#include <string>

namespace {
   static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD
{
    // cppcheck-suppress unknownMacro
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint8_t, threshold, setThreshold )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint8_t, candCharge, setCandCharge )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint8_t, mdtFlag, setMdtFlag )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint16_t, subdetectorId, setSubdetectorId )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint16_t, sectorId, setSectorId )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, uint16_t, bcTag, setBcTag )
    IMPLEMENT_SETTER_GETTER( ICandData_v1, xAOD::ICandData_v1::Quality, candQuality, setCandQuality )

    void ICandData_v1::setEta(float eta) {
        float etaClamped = std::clamp(eta, -s_etaRange, s_etaRange);
        uint16_t etaBinary = static_cast<uint16_t>(std::lround((etaClamped + s_etaRange) / (2.0f * s_etaRange) * static_cast<float>(s_etaBitRange)));
        static const SG::Accessor<uint16_t> acc(preFixStr + "eta");
        acc(*this) = etaBinary;
    }

    uint16_t ICandData_v1::eta() const {
        static const SG::Accessor<uint16_t> acc(preFixStr + "eta");
        return  acc(*this);
    }

    void ICandData_v1::setPhi(float phi) {
        uint16_t phiBinary = static_cast<uint16_t>(((phi + M_PI) / s_phiRange) * static_cast<float>(s_phiBitRange));
        static const SG::Accessor<uint16_t> acc(preFixStr + "phi");
        acc(*this) = phiBinary;
    }

    uint16_t ICandData_v1::phi() const{
        static const SG::Accessor<uint16_t> acc(preFixStr + "phi");
        //return (static_cast<float>(acc(*this)) / static_cast<float>(s_phiBitRange)) * s_phiRange-M_PI;
        return  acc(*this);
    }

    void ICandData_v1::setPt(float pt){
        const float ptClamped = std::clamp(pt, 0.0F, s_ptRange);
        const uint8_t ptBinary = static_cast<uint8_t>(
            std::lround(ptClamped / s_ptResolution));
        static const SG::Accessor<uint8_t> acc(preFixStr + "pt");
        acc(*this) = ptBinary;
    }

    uint8_t ICandData_v1::pt() const{
        static const SG::Accessor<uint8_t> acc(preFixStr + "pt");
        return  acc(*this);
    }

    void ICandData_v1::setCoinType(uint8_t cointype) {
        uint8_t coinTypeBin = cointype & COINTYPE_BIT_MASK;
        static const SG::Accessor<uint8_t> acc(preFixStr + "coinType");
        acc(*this) = coinTypeBin;
    }

    uint8_t ICandData_v1::coinType() const {
        static const SG::Accessor<uint8_t> acc(preFixStr + "coinType");
        return  acc(*this);
    }

    void ICandData_v1::initialize(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag) {
      setSubdetectorId(subdetectorId);
      setSectorId(sectorId);
      setBcTag(bcTag);
  }

} // namespace xAOD
