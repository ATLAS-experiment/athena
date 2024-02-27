/**
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 *
 * @file HGTDCluster_v1.h
 * @author Dimitrios Ntounis <dimitrios.ntounis@cern.ch>
 * @date 2024
 * @brief Class to represent HGTDCluster_v1 in the xAOD format.
 */

#ifndef XAODINDETMEASUREMENT_HGTDCLUSTER_V1_H
#define XAODINDETMEASUREMENT_HGTDCLUSTER_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {

/// @class PixelCluster_v1
/// Class describing HGTD clusters


class HGTDCluster_v1 : public UncalibratedMeasurement_v1 {

    public:

    /// Default constructor
    HGTDCluster_v1() = default;
    /// Virtual destructor
    virtual ~HGTDCluster_v1() = default;


    /// Returns the type of the HGTD cluster as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::HGTDClusterType;
    }
    unsigned int numDimensions() const override final { return 3; }


    /// Returns the list of identifiers of the channels building the cluster
    const std::vector<Identifier> rdoList() const;



    /// Returns the list of Time Over Threshold of the channels building the cluster
    const std::vector<int>& totList() const;


    /// @name Functions to set HGTD cluster properties
    /// @{

    /// Sets the list of identifiers of the channels building the cluster
    void setRDOlist(const std::vector<Identifier>& rdolist);

    /// Sets the list of ToT of the channels building the cluster
    void setToTlist(const std::vector<int>& tots);

    /// @}

};

} // namespace xAOD
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::HGTDCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif // XAODHGTDCluster_v1_H
