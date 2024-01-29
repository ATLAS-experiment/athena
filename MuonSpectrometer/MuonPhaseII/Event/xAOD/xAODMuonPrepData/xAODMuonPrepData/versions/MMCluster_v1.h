/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MMCluster_V1_H
#define XAODMUONPREPDATA_VERSION_MMCluster_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "CxxUtils/CachedValue.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonPrepRawData/MMPrepData.h"
namespace xAOD {

class MMCluster_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    MMCluster_v1() = default;
    /// Virtual destructor
    virtual ~MMCluster_v1() = default;

    /// Returns the type of the MM strip as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::MMClusterType;
    }
    unsigned int numDimensions() const override final { return 1; }

    /** @brief  Returns the gas gap number to which the clsuter*/
    uint8_t gasGap() const;

    void setGasGap(uint8_t gap);
    /** @brief returns the number of the central strip*/
    uint16_t channelNumber() const;

    void setChannelNumber(uint16_t strip);
    
    /** @brief Returns the hash of the measurement channel*/
    IdentifierHash measurementHash() const;

    /** @brief Returns the time  (ns). 
    The time is calibrated, i.e. it is in units of ns, after t0 subtraction.*/
    uint16_t time() const;
    /** @brief Sets the TDC counts */
    void setTime(uint16_t value);

    /** @brief Returns the charge
     * The charge is calibrated, i.e. it is in units of electrons, after pedestal subtraction.
    */
    uint32_t charge() const;
    /** @brief Sets the calibrated charge */
    void setCharge(uint32_t value);

    /** @brief Returns the Drift Distance*/
    float driftDist() const;
    /** @brief Sets the drift distance */
    void setDriftDist(float value);

    /** @brief Returns the microTPC angle */
    float angle() const;
    /** @brief Sets the microTPC angle*/
    void setAngle(float value);

    /** @brief Returns the microTPC chisq Prob. */
    float chiSqProb() const;
    /** @brief Sets the microTPC chisq probability*/
    void setChiSqProb(float value);

    using Author = Muon::MMPrepData::Author;
    Author author() const;
    void setAuthor(Author author);
    
    using Quality = Muon::MMPrepData::Quality;
    Quality quality() const;
    void setQuality(Quality quality);

    /** @brief returns the list of strip numbers */
    const std::vector<uint16_t>& stripNumbers() const;
    void setStripNumbers(const std::vector<uint16_t>& stripNumbers);

    /** @brief returns the list of times */
    const std::vector<int16_t>& stripTimes() const;
    void setStripTimes(const std::vector<int16_t>& stripTimes);

    /** @brief returns the list of charges */
    const std::vector<int>& stripCharges() const;
    void setStripCharges(const std::vector<int>& stripCharges);

    /** @brief returns the list of drift distances */
    const std::vector<float>& stripDriftDist() const;
    void setStripDriftDist(const std::vector<float>& stripDriftDist);

    /** @brief returns the list of drift distances */
    using DriftCov_t = PosAccessor<2>::element_type;
    const std::vector<DriftCov_t>& stripDriftErrors() const;

    void setStripDriftErrors(const std::vector<DriftCov_t>& stripDriftErrors);
    void setStripDriftErrors(const std::vector<Amg::MatrixX>& stripDriftErrors);

    /** @brief set the pointer to the MmReadoutElement */
    void setReadoutElement(const MuonGMR4::MmReadoutElement* readoutEle);
    /** @brief Retrieve the associated MmReadoutElement. 
        If the element has not been set before, it's tried to load it on the fly. 
        Exceptions are thrown if that fails as well */
    const MuonGMR4::MmReadoutElement* readoutElement() const;

    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::MmReadoutElement *>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::MmReadoutElement *> m_readoutEle{};
#endif


};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::MMCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif