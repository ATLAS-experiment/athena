/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/versions/IL1CandData_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include <cmath>
#include <algorithm>

namespace xAOD {
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( IL1CandData_v1, uint8_t, l1Threshold, setL1Threshold )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( IL1CandData_v1, uint8_t, l1CandCharge, setL1CandCharge )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( IL1CandData_v1, uint16_t, l1SubdetectorId, setL1SubdetectorId )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( IL1CandData_v1, uint16_t, l1SectorId, setL1SectorId )
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( IL1CandData_v1, uint16_t, bcTag, setBcTag )

    void IL1CandData_v1::setL1Eta(float eta) {
        float etaClamped = std::clamp(eta, -s_etaRange, s_etaRange);
        uint16_t etaBinary = static_cast<uint16_t>(std::lround((etaClamped + s_etaRange) / (2.0f * s_etaRange) * static_cast<float>(s_etaBitRange)));
        static const SG::AuxElement::Accessor<uint16_t> acc("l1Eta");
        acc(*this) = etaBinary;
    }

    uint16_t IL1CandData_v1::l1Eta() const {
        static const SG::AuxElement::Accessor<uint16_t> acc("l1Eta");
        return  acc(*this);
    }

    void IL1CandData_v1::setL1Phi(float phi) {
        uint16_t phiBinary = static_cast<uint16_t>(((phi + M_PI) / s_phiRange) * static_cast<float>(s_phiBitRange));
        static const SG::AuxElement::Accessor<uint16_t> acc("l1Phi");
        acc(*this) = phiBinary;
    }

    uint16_t IL1CandData_v1::l1Phi() const{
        static const SG::AuxElement::Accessor<uint16_t> acc("l1Phi");
        //return (static_cast<float>(acc(*this)) / static_cast<float>(s_phiBitRange)) * s_phiRange-M_PI;
        return  acc(*this);
    }

    void IL1CandData_v1::setL1Pt(float pt){
        const float ptClamped = std::clamp(pt, 0.0F, s_ptRange);
        const uint8_t ptBinary = static_cast<uint8_t>(
            std::lround(ptClamped / s_ptResolution));
        static const SG::AuxElement::Accessor<uint8_t> acc("l1Pt");
        acc(*this) = ptBinary;
    }

    uint8_t IL1CandData_v1::l1Pt() const{
        static const SG::AuxElement::Accessor<uint8_t> acc("l1Pt");
        return  acc(*this);
    }

    void IL1CandData_v1::setCoinType(uint8_t cointype) {
        uint8_t coinTypeBin = cointype & COINTYPE_BIT_MASK;
        static const SG::AuxElement::Accessor<uint8_t> acc("coinType");
        acc(*this) = coinTypeBin;
    }

    uint8_t IL1CandData_v1::coinType() const {
        static const SG::AuxElement::Accessor<uint8_t> acc("coinType");
        return  acc(*this);
    }

    void IL1CandData_v1::setTcId(uint8_t value) {
        static const SG::Accessor<uint8_t> acc("tcId");
        acc(*this) = value & TC_ID_BIT_MASK;
    }

    uint8_t IL1CandData_v1::tcId() const {
        static const SG::ConstAccessor<uint8_t> acc("tcId");
        return acc(*this) & TC_ID_BIT_MASK;
    }

    void IL1CandData_v1::initialize(uint16_t subdetectorId, uint16_t sectorId, uint16_t bcTag) {
      setL1SubdetectorId(subdetectorId);
      setL1SectorId(sectorId);
      setBcTag(bcTag);
  }

} // namespace xAOD
