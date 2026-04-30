/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCMEASUREMENT_V1_H
#define XAODMUONPREPDATA_VERSION_RPCMEASUREMENT_V1_H

#include "xAODMuonPrepData/versions/MuonMeasurement_v1.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"


namespace xAOD {

/** @brief RpcMeasurement_v1: Class to store the common information for
 *                            RpcMeasurements 
*/

class RpcMeasurement_v1 : public MuonMeasurement_v1 {

   public:
    /// Default constructor
    RpcMeasurement_v1() = default;
    /// Virtual destructor
    virtual ~RpcMeasurement_v1() = default;

    /// Returns the type of the Rpc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::RpcStripType;
    }
    /** @brief Returns the local position of the measurement */
    Amg::Vector3D localMeasurementPos() const override final;
    
    /** @brief returns the associated strip number*/
    std::uint16_t channelNumber() const;
    /** @brief returns the associated gas gap */
    std::uint8_t gasGap() const;
    /** @brief doubletPhi identifier field of the measurement */
    std::uint8_t doubletPhi() const;

    /** @brief Returns the time. */
    float time() const;
    /** @brief Returns the uncertainty squared on the time measurement */
    float timeCovariance() const;

    /** @brief Returns the trigger coincidence - usually false, unless ijk>5 or highpt&&ijk==0*/
    std::uint32_t triggerInfo() const;

    /** @brief Returns the number of ambiguities associated with this RpcPrepData.
        - 0 if the ambiguites have not been removed by choice;
        - 1 if the ambiguities are fully solved
        - i+1 if "i" other MuonPrepRawData are produced along with the current one from a single RDO hit*/
    std::uint8_t ambiguityFlag() const;

    /** @brief Returns the time over threshold */
    float timeOverThreshold() const;
    /** @brief Returns the hash of the measurement channel */
    IdentifierHash measurementHash() const override final;
    /** @brief Returns the hash of the associated layer (Needed for surface retrieval)*/
    IdentifierHash layerHash() const override final;

    /** @brief Retrieve the associated RpcReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::RpcReadoutElement* readoutElement() const override final;

    /** @brief Sets the the triger time of the hit */
    void setTime(float time);
    /** @brief Set the trigger info of the hit  */
    void setTriggerInfo(std::uint32_t triggerinfo);
    /** @brief Sets the ADC counts */
    void setAmbiguityFlag(std::uint8_t ambi);
    /** @brief Sets the TDC counts */
    void setTimeOverThreshold(float timeoverthreshold);
    /** @brief Sets the doubletPhi identifier field */
    void setDoubletPhi(std::uint8_t doubPhi);
    /** @brief Sets the  associated gasGap identifier field */
    void setGasGap(std::uint8_t gap);
    /** @brief Sets the associated strip number identifier field */
    void setChannelNumber(std::uint16_t strip);
    /** @brief set the pointer to the ReadoutElement */
    void setReadoutElement(const MuonGMR4::RpcReadoutElement* readoutEle);
    /** @brief Set the time covariance of the Measurement */
    void setTimeCovariance(float timeCov);
};

}  // namespace xAOD


#endif
