/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H
#define XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "xAODCore/VariableStruct.h"

namespace xAOD {

/// @class PixelCluster_v1
/// Class describing pixel clusters

class PixelCluster_v1 : public UncalibratedMeasurement_v1 {

   public:
    /// Default constructor
    PixelCluster_v1() = default;
    /// Virtual destructor
    virtual ~PixelCluster_v1() = default;

    /// @name Functions to get pixel cluster properties
    /// @{

    /// Returns the type of the pixel cluster as a simple enumeration
    xAOD::UncalibMeasType type() const override final {
        return xAOD::UncalibMeasType::PixelClusterType;
    }
    unsigned int numDimensions() const override final { return 2; }

    /// Returns the global position of the pixel cluster
    ConstVectorMap<3> globalPosition() const;
    VectorMap<3> globalPosition();

    /// Returns the list of identifiers of the channels building the cluster
    const std::vector<Identifier> rdoList() const;

    /// Returns the dimensions of the cluster in numbers of channels in phi (x)
    /// and eta (y) directions, respectively
    int channelsInPhi() const;
    int channelsInEta() const;

    /// Returns the width of the cluster in phi (x) and eta (y) directions,
    /// respectively
    float widthInEta() const;

    /// Returns the list of ToT of the channels building the cluster
    const std::vector<int>& totList() const;

    /// Returns the list of charges of the channels building the cluster
    const std::vector<float>& chargeList() const;

    /// Return the energy loss in the cluster in MeV
    float energyLoss() const;

    /// Returns if the cluster is split or not
    bool isSplit() const;

    /// Returns the splitting probabilities for the cluster
    float splitProbability1() const;
    float splitProbability2() const;

    /// Return the LVL1 accept
    int lvl1a() const;

    /// @}

    /// @name Functions to set pixel cluster properties
    /// @{

    /// Sets the list of identifiers of the channels building the cluster
    void setRDOlist(const std::vector<Identifier>& rdolist);

    /// Setter with std::move if the value_type is already available
    void setRDOlist(std::vector<Identifier::value_type>&& rdolist);

    /// Sets the dimensions of the cluster in numbers of channels in phi (x) and
    /// eta (y) directions
    void setChannelsInPhiEta(int channelsInPhi, int channelsInEta);

    /// Sets the width of the cluster in eta (y) direction
    void setWidthInEta(float widthInEta);

    /// Sets the list of ToT of the channels building the cluster
    void setToTlist(const std::vector<int>& tots);
    void setToTlist(std::vector<int>&& tots);

    /// Sets the list of charges of the channels building the cluster
    void setChargelist(const std::vector<float>& charges);
    void setChargelist(std::vector<float>&& charges);

    /// Sets the energy loss in the cluster in MeV
    void setEnergyLoss(float dEdX);

    /// Sets if the cluster is split or not
    void setIsSplit(bool isSplit);

    /// Sets the splitting probabilities for the cluster
    void setSplitProbabilities(float prob1, float prob2);

    /// Sets the LVL1 accept
    void setLVL1A(int lvl1a);

    /// @}

    /// @name Create a structure of raw pointers for fast filling.
    /// @{

    struct ClusterVars : public xAOD::VariableStruct
    {
      using xAOD::VariableStruct::VariableStruct;

      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIdentType,              identifier);
      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIDHashType,             identifierHash);
      AUXSTORE_VARSTRUCT_VAR(xAOD::PosAccessor<2>::element_type,   localPositionDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::CovAccessor<2>::element_type,   localCovarianceDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::PosAccessor<3>::element_type,   globalPosition);
      AUXSTORE_VARSTRUCT_VAR(std::vector<Identifier::value_type>,  rdoList);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInPhi);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInEta);
      AUXSTORE_VARSTRUCT_VAR(float,                                widthInEta);
      AUXSTORE_VARSTRUCT_VAR(std::vector<int>,                     totList);
      AUXSTORE_VARSTRUCT_VAR(int,                                  lvl1a);
    };

    /// @}
};

}  // namespace xAOD
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::PixelCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif
