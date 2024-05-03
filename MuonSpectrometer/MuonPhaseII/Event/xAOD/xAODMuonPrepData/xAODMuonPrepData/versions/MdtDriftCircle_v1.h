/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MDTDRIFTCIRCLE_V1_H
#define XAODMUONPREPDATA_VERSION_MDTDRIFTCIRCLE_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "MuonPrepRawData/MdtDriftCircleStatus.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "CxxUtils/CachedValue.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"

namespace xAOD {
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonPrepRawData/MuonPrepRawData/MdtPrepData.h

class MdtDriftCircle_v1 : public UncalibratedMeasurement_v1 {

   public:
    using MdtDriftCircleStatus = Muon::MdtDriftCircleStatus;
    /// Default constructor
    MdtDriftCircle_v1() = default;
    /// Virtual destructor
    virtual ~MdtDriftCircle_v1() = default;

    /// Returns the type of the Mdt drift circle as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::MdtDriftCircleType;
    }
    unsigned int numDimensions() const override final { return 1; }
    /** @brief Returns the TDC (typically range is 0 to 2500)*/
    int16_t tdc() const;
    /** @brief Returns the ADC (typically range is 0 to 250)*/
    int16_t adc() const;

    /** @brief Returns the tube number of the measurement (1-120)*/
    uint16_t driftTube() const;
    /** @brief Returns the layer number of the measurement (1-4)*/
    uint8_t tubeLayer() const;
    /** @brief Returns the hash of the measurement channel (tube (x) layer) */
    IdentifierHash measurementHash() const;
    /** @brief Returns the status of the measurement */
    MdtDriftCircleStatus status() const;
    /** @brief Returns the drift radius*/
    float driftRadius() const;
    /** @brief Returns the covariance of the drift radius*/
    float driftRadiusCov() const;
    /** @brief Returns the uncertainty on the drift radius*/
    float driftRadiusUncert() const;

    /// Setter methods

    /** @brief Sets the TDC counts */
    void setTdc(int16_t tdc);
    /** @brief Sets the ADC counts */
    void setAdc(int16_t adc);
    /** @brief Sets the tube number */
    void setTube(uint16_t tube_n);
    /** @brief Sets the layer number */
    void setLayer(uint8_t layer_n);
    /** @brief Sets the status of the drift circle */
    void setStatus(MdtDriftCircleStatus st);
    /** @brief Sets the drift radius of the drift circle */
    void setDriftRadius(float r);
    /** @brief Sets the covariance on the drift circle */
    void setDriftRadCov(float cov);

    /** @brief set the pointer to the MdtReadoutElement */
    void setReadoutElement(const MuonGMR4::MdtReadoutElement* readoutEle);
    /** @brief Retrieve the associated MdtReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::MdtReadoutElement* readoutElement() const;

    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::MdtReadoutElement *>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::MdtReadoutElement *> m_readoutEle{};
#endif

};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::MdtDriftCircle_v1, xAOD::UncalibratedMeasurement_v1);
#endif