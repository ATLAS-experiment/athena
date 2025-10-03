/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_STRIPCLUSTER_V1_H
#define XAODINDETMEASUREMENT_VERSION_STRIPCLUSTER_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {

/// @class StripCluster_v1
/// Class describing strip clusters

class StripCluster_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    StripCluster_v1() = default;
    /// Virtual destructor
    virtual ~StripCluster_v1() = default;

    /// @name Functions to get strip cluster properties
    /// @{

    /// Returns the type of the strip cluster as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::StripClusterType;
    }
    unsigned int numDimensions() const override final { return 1; }

    /// Returns the global position of the strip cluster
    ConstVectorMap<3> globalPosition() const;
    VectorMap<3> globalPosition();

    /// Returns the list of identifiers of the channels building the cluster
    const std::vector<Identifier> rdoList() const;

    /// Returns the dimensions of the cluster in numbers of channels in phi (x),
    /// respectively
    int channelsInPhi() const;

    /// @}

    /// @name Functions to set pixel cluster properties
    /// @{
  
    /// Sets the list of identifiers of the channels building the cluster
    void setRDOlist(const std::vector<Identifier>& rdolist);
    /// Setter with std::move if the value_type is already available
    void setRDOlist(std::vector<Identifier::value_type>&& rdolist);
    /// Sets the dimensions of the cluster in numbers of channels in phi (x)
    void setChannelsInPhi(int channelsInPhi);

    /// @}
};

}  // namespace xAOD
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::StripCluster_v1, xAOD::UncalibratedMeasurement_v1);

#endif
