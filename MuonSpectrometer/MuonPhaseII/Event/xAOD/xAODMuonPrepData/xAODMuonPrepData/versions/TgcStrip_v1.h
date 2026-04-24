/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H

#include "xAODMuonPrepData/versions/MuonMeasurement_v1.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"

namespace xAOD {

class TgcStrip_v1 : public MuonMeasurement_v1 {

   public:
    /// Default constructor
    TgcStrip_v1() = default;
    /// Virtual destructor
    virtual ~TgcStrip_v1() = default;

    /// Returns the type of the Tgc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::TgcStripType;
    }
    unsigned numDimensions() const override final { return 1; }

    /** @brief Returns the bcBitMap of this PRD
      bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
    std::uint8_t bcBitMap() const;
    /** @brief Set the bunch crossing-id map */
    void setBcBitMap(std::uint8_t bitMap);

    /** @brief Strip or wire group number of the Tgc strip measurement*/
    std::uint16_t channelNumber() const;
    /** @brief Set the strip or wire group number of the measurement */
    void setChannelNumber(std::uint16_t chan);

    /** @brief Associated gas gap number of the Tgc strip measurement 
     *         Ranges [1-N]*/
    std::uint8_t gasGap() const;
    /** @brief Set the gas gap number of the measurement [1-N] */
    void setGasGap(uint8_t gapNum);
    /** @brief Returns the local position of the measurement */
    Amg::Vector3D localMeasurementPos() const override final;
    
    /**  @brief Does the object belong to an eta or a phi measurement (si /no) */
    std::uint8_t measuresPhi() const override final;
    /** @brief Set the measures phi flag of the measurement to true /false */
    void setMeasuresPhi(std::uint8_t measPhi);

    /** @brief Returns the hash of the measurement channel  */
    IdentifierHash measurementHash() const override final;
    /** @brief Returns the hash of the associated layer (Needed for surface retrieval)*/
    IdentifierHash layerHash() const override final;

    /** @brief set the pointer to the TgcReadoutElement */
    void setReadoutElement(const MuonGMR4::TgcReadoutElement* readoutEle);
    /** @brief Retrieve the associated TgcReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::TgcReadoutElement* readoutElement() const override final;
};

}  // namespace xAOD

#endif