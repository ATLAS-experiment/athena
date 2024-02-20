/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_TGCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "CxxUtils/CachedValue.h"

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

    /** @brief Returns the bcBitMap of this PRD
      bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
    uint16_t bcBitMap() const;

    void setBcBitMap(uint16_t);

    /** @brief Strip or wire group number of the Tgc strip measurement*/
    uint16_t channelNumber() const;
    
    void setChannelNumber(uint16_t chan);

    /** @brief Associated gas gap number of the Tgc strip measurement 
     *         Ranges [1-N]
    */
    uint8_t gasGap() const;
    
    void setGasGap(uint8_t gapNum);
    
    /**  @brief Does the object belong to an eta or a phi measurement (si /no) */
    uint8_t measuresPhi() const;

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
    /** @brief Returns the local position of the strip within
     *        the Muon Station (I.e. the center plane between 2 multilayers)
     */
    ConstVectorMap<3> stripPosInStation() const;

    /** @brief Sets the position of the strip within a muon station*/
    void setStripPosInStation(const MeasVector<3>& pos);
    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::TgcReadoutElement*>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::TgcReadoutElement*> m_readoutEle{};
#endif



};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::TgcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif