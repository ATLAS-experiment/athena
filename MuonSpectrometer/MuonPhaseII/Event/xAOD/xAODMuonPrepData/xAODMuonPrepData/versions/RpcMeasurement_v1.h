/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCMEASUREMENT_V1_H
#define XAODMUONPREPDATA_VERSION_RPCMEASUREMENT_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace MuonGMR4{
    class RpcReadoutElement;
}
namespace xAOD {

/** @brief RpcMeasurement_v1: Class storing the geneic 
 * 
*/

class RpcMeasurement_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    RpcMeasurement_v1() = default;
    /// Virtual destructor
    virtual ~RpcMeasurement_v1() = default;

    /// Returns the type of the Rpc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::RpcStripType;
    }
    /** @brief: Returns the Athena identifier of the measurement It's constructed from the measurementHash
     *          which is passed to the associated readoutElement */
    const Identifier& identify() const;
    /** @brief Returns the local position of the measurement */
    Amg::Vector3D localMeasurementPos() const;
    
    /** @brief returns the associated strip number*/
    uint16_t stripNumber() const;
    /** @brief returns the associated gas gap */
    uint8_t gasGap() const;
    /** @brief returns whether the hit measures the phi coordinate */
    virtual uint8_t measuresPhi() const = 0;
    /** @brief doubletPhi identifier field of the measurement */
    uint8_t doubletPhi() const;

    /** @brief Returns the time. */
    float time() const;
    /** @brief Returns the uncertainty squared on the time measurement */
    float timeCovariance() const;

    /** @brief Returns the trigger coincidence - usually false, unless ijk>5 or highpt&&ijk==0*/
    uint32_t triggerInfo() const;

    /** @brief Returns the number of ambiguities associated with this RpcPrepData.
        - 0 if the ambiguites have not been removed by choice;
        - 1 if the ambiguities are fully solved
        - i+1 if "i" other MuonPrepRawData are produced along with the current one from a single RDO hit*/
    uint8_t ambiguityFlag() const;

    /** @brief Returns the time over threshold */
    float timeOverThreshold() const;
    /** @brief Returns the hash of the measurement channel */
    IdentifierHash measurementHash() const;
    /** @brief Returns the hash of the associated layer (Needed for surface retrieval)*/
    IdentifierHash layerHash() const;

    /** @brief Retrieve the associated RpcReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::RpcReadoutElement* readoutElement() const;

    /** @brief Sets the the triger time of the hit */
    void setTime(float time);
    /** @brief Set the trigger info of the hit  */
    void setTriggerInfo(uint32_t triggerinfo);
    /** @brief Sets the ADC counts */
    void setAmbiguityFlag(uint8_t ambi);
    /** @brief Sets the TDC counts */
    void setTimeOverThreshold(float timeoverthreshold);
    /** @brief Sets the doubletPhi identifier field */
    void setDoubletPhi(uint8_t doubPhi);
    /** @brief Sets the  associated gasGap identifier field */
    void setGasGap(uint8_t gap);
    /** @brief Sets the associated strip number identifier field */
    void setStripNumber(uint16_t strip);
    /** @brief set the pointer to the ReadoutElement */
    void setReadoutElement(const MuonGMR4::RpcReadoutElement* readoutEle);
    /** @brief Set the time covariance of the Measurement */
    void setTimeCovariance(float timeCov);
    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::RpcReadoutElement*>)]{};
    char m_identifier[sizeof(CxxUtils::CachedValue<Identifier>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::RpcReadoutElement*> m_readoutEle{};
    CxxUtils::CachedValue<Identifier> m_identifier{};
#endif

};

}  // namespace xAOD


#endif
