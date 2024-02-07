/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCSTRIPCLUSTER_V1_H
#define XAODMUONPREPDATA_VERSION_STGCSTRIPCLUSTER_V1_H

#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"

namespace xAOD {


class sTgcStripCluster_v1 : public sTgcMeasurement_v1 {

  public:
    /// Default constructor
    sTgcStripCluster_v1() = default;
    /// Virtual destructor
    virtual ~sTgcStripCluster_v1() = default;

    /// Returns the type of the Tgc strip as a simple enumeration
    sTgcChannelTypes channelType() const override final {
      return sTgcChannelTypes::Strip;
    }

    /** @brief returns the list of strip numbers building up the cluster */
    const std::vector<uint16_t>& stripNumbers() const;
    /** @brief returns the list of invidvidual channel times building up the cluster */
    const std::vector<short int>& stripTimes() const;
    /** @brief returns the list of the inidivudal channel charges bulding up the cluster*/
    const std::vector<int>& stripCharges() const;

    /** @brief Set the list of individual strip clusters */
    void setStripNumbers(const std::vector<uint16_t>& strips);
    /** @brief Set the list of indivual strip times */
    void setStripTimes(const std::vector<short int>&  times);
    /** @brief Set the individual list of contributing charges*/
    void setStripCharges(const std::vector<int>& charges);

    
    /** @brief Which quality flag does the Measurement pass*/
    using Quality = Muon::sTgcPrepData::Quality;
    Quality quality() const;
    /** @brief Set the quality flag */
    void setQuality(Quality q);


};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE_FIN(xAOD::sTgcStripCluster_v1, xAOD::sTgcMeasurement_v1);

#endif