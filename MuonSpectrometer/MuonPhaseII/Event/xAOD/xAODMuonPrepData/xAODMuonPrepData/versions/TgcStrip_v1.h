/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "CxxUtils/CachedValue.h"

namespace MuonGMR4{
    class TgcReadoutElement;
}

namespace xAOD {

class TgcStrip_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    TgcStrip_v1() = default;
    /// Virtual destructor
    virtual ~TgcStrip_v1() = default;

    /// Returns the type of the Tgc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::TgcStripType;
    }
    unsigned int numDimensions() const override final { return 1; }

    /** @brief: Returns the Athena identifier of the measurement
      *         It's constructed from the measurementHash & passed to the associated readoutElement */
    const Identifier& identify() const;
    /** @brief Returns the bcBitMap of this PRD
      bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
    uint16_t bcBitMap() const;
    /** @brief Set the bunch crossing-id map */
    void setBcBitMap(uint16_t);

    /** @brief Strip or wire group number of the Tgc strip measurement*/
    uint16_t channelNumber() const;
    /** @brief Set the strip or wire group number of the measurement */
    void setChannelNumber(uint16_t chan);

    /** @brief Associated gas gap number of the Tgc strip measurement 
     *         Ranges [1-N]*/
    uint8_t gasGap() const;
    /** @brief Set the gas gap number of the measurement [1-N] */
    void setGasGap(uint8_t gapNum);
    /** @brief Returns the local position of the measurement */
    Amg::Vector3D localMeasurementPos() const;
    
    /**  @brief Does the object belong to an eta or a phi measurement (si /no) */
    uint8_t measuresPhi() const;
    /** @brief Set the measures phi flag of the measurement to true /false */
    void setMeasuresPhi(uint8_t measPhi);

    /** @brief Returns the hash of the measurement channel  */
    IdentifierHash measurementHash() const;
    /** @brief Returns the hash of the associated layer (Needed for surface retrieval)*/
    IdentifierHash layerHash() const;

    /** @brief set the pointer to the TgcReadoutElement */
    void setReadoutElement(const MuonGMR4::TgcReadoutElement* readoutEle);
    /** @brief Retrieve the associated TgcReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::TgcReadoutElement* readoutElement() const;

    private:
        CxxUtils::CachedValue<const MuonGMR4::TgcReadoutElement*> m_readoutEle{};
        CxxUtils::CachedValue<Identifier> m_identifier{};
};

}  // namespace xAOD

#endif