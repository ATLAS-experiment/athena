/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MDTTWINDRIFTCIRCLE_V1_H
#define XAODMUONPREPDATA_VERSION_MDTTWINDRIFTCIRCLE_V1_H

#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"

namespace xAOD {


class MdtTwinDriftCircle_v1 : public MdtDriftCircle_v1 {

   public:
    /// Default constructor
    MdtTwinDriftCircle_v1() = default;
    /// Virtual destructor
    virtual ~MdtTwinDriftCircle_v1() = default;

    unsigned int numDimensions() const override final { return 2; }
    
    /** @brief Returns the Identifier of the twin tube */
    Identifier twinIdentify() const;
    /** @brief Returns the coordinate along the wire */
    float posAlongWire() const;
    /** @brief Returns the covariance of the coordinate along the wire */
    float posAlongWireCov() const;
    /** @brief Returns the uncertainty on the coordinate along the wire */
    float posAlongWireUncert() const;
    /** @brief Returns the TDC (typically range is 0 to 2500)*/
    int16_t twinTdc() const;
    /** @brief Returns the ADC (typically range is 0 to 250)*/
    int16_t twinAdc() const;
    /** @brief Returns the tube number of the associated twin channel (1-120)*/
    uint16_t twinTube() const;
    /** @brief Returns the layer number of the associated twin channel (1-4)*/
    uint8_t twinLayer() const;   
    /** @brief Sets the TDC counts */
    void setTwinTdc(int16_t tdc);
    /** @brief Sets the ADC counts */
    void setTwinAdc(int16_t adc);
    /** @brief Sets the tube number */
    void setTwinTube(uint16_t tube_n);
    /** @brief Sets the layer number */
    void setTwinLayer(uint8_t layer_n);
};

}  // namespace xAOD

#endif