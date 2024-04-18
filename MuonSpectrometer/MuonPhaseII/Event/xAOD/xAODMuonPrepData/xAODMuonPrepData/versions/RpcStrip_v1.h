/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
namespace xAOD {

class RpcStrip_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    RpcStrip_v1() = default;
    /// Virtual destructor
    virtual ~RpcStrip_v1() = default;

    /// Returns the type of the Rpc strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::RpcStripType;
    }
    unsigned int numDimensions() const override final { return 1; }
    
    /** @brief returns the associated strip number*/
    uint16_t stripNumber() const;

    void setStripNumber(uint16_t strip);
    /** @brief returns the associated gas gap */
    uint8_t gasGap() const;

    void setGasGap(uint8_t gap);
    /** @brief returns whether the hit measures the phi coordinate */
    uint8_t measuresPhi() const;

    void setMeasuresPhi(uint8_t measPhi);
     /** @brief doubletPhi */
    uint8_t doubletPhi() const;

    void setDoubletPhi(uint8_t doubPhi);
    
    /** @brief Returns the time. */
    float time() const;

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
    /** @brief Sets the the triger time of the hit */
    void setTime(float time);
    /** @brief Set the trigger info of the hit  */
    void setTriggerInfo(uint32_t triggerinfo);
    /** @brief Sets the ADC counts */
    void setAmbiguityFlag(uint8_t ambi);
    /** @brief Sets the TDC counts */
    void setTimeOverThreshold(float timeoverthreshold);

    /** @brief set the pointer to the ReadoutElement */
    void setReadoutElement(const MuonGMR4::RpcReadoutElement* readoutEle);
    /** @brief Retrieve the associated MdtReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::RpcReadoutElement* readoutElement() const;

    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::RpcReadoutElement*>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::RpcReadoutElement*> m_readoutEle{};
#endif

};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::RpcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif
