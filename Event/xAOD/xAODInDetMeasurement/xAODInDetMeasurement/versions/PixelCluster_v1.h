/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H
#define XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "xAODInDetMeasurement/JaggedVecEltCache.h"
#include "AthContainers/JaggedVecAccessor.h"
#include "xAODCore/VariableStruct.h"
#include "xAODInDetMeasurement/ArrayFloat3.h"

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
    SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
    rdoList() const;

    /// Returns the dimensions of the cluster in numbers of channels in phi (x)
    /// and eta (y) directions, respectively
    int channelsInPhi() const;
    int channelsInEta() const;

    /// Returns the width of the cluster in phi (x) and eta (y) directions,
    /// respectively
    float widthInEta() const;

    /// Returns the list of ToT of the channels building the cluster
    SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
    totList() const;

    /// Returns the list of charges of the channels building the cluster
    SG::ConstAccessor<SG::JaggedVecElt<float> >::element_type
    chargeList() const;
    /// Returns the sum of the charges of the channels building the cluster
    float totalCharge() const;

    /// Return the energy loss in the cluster in MeV
    float energyLoss() const;

    /// Return the LVL1 accept
    int lvl1a() const;

    /// @}

    /// @name Functions to set pixel cluster properties
    /// @{

    /// Sets the list of identifiers of the channels building the cluster
    void setRDOlist(const std::vector<Identifier>& rdolist);

    /// Setter with std::move if the value_type is already available
    void setRDOlist(std::vector<Identifier::value_type>&& rdolist);

    /// Setter with std::move if the value_type is already available
    void setRDOlist(std::span<Identifier::value_type> rdolist);

    /// Sets the dimensions of the cluster in numbers of channels in phi (x) and
    /// eta (y) directions
    void setChannelsInPhiEta(int channelsInPhi, int channelsInEta);

    /// Sets the width of the cluster in eta (y) direction
    void setWidthInEta(float widthInEta);

    /// Sets the list of ToT of the channels building the cluster
    void setToTlist(const std::vector<int>& tots);
    void setToTlist(std::span<int> tots);

    /// Sets the list of charges of the channels building the cluster
    void setChargelist(const std::vector<float>& charges);
    void setChargelist(std::span<float> charges);
    /// Sets the total charge
    void setTotalCharge(float totalCharge);

    /// Sets the energy loss in the cluster in MeV
    void setEnergyLoss(float dEdX);

    /// Sets the LVL1 accept
    void setLVL1A(int lvl1a);

    static const SG::AuxElement::Accessor<SG::JaggedVecElt<Identifier::value_type> > rdoListAcc() { return s_rdoListAcc; }
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<int> > totListAcc() { return s_totListAcc; }
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<float> > chargeListAcc() { return s_chargeListAcc; }
    /// @}
protected:
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<Identifier::value_type> > s_rdoListAcc;
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<int> > s_totListAcc;
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<float> > s_chargeListAcc;
public:
    /// @name Create a structure of raw pointers for fast filling.
    /// @{

    struct ClusterVars : public xAOD::VariableStruct
    {
      ClusterVars(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
         : xAOD::VariableStruct(cont),
           rdoList(cont, s_rdoListAcc, n_cluster_rdos),
           totList(cont, s_totListAcc, n_cluster_rdos),
           chargeList(cont, s_chargeListAcc, n_cluster_rdos)
       {}

      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIdentType,              identifier);
      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIDHashType,             identifierHash);
      AUXSTORE_VARSTRUCT_VAR(xAOD::PosAccessor<2>::element_type,   localPositionDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::CovAccessor<2>::element_type,   localCovarianceDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::ArrayFloat3,                    globalPosition);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInPhi);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInEta);
      AUXSTORE_VARSTRUCT_VAR(float,                                widthInEta);
      AUXSTORE_VARSTRUCT_VAR(float,                                totalCharge);
      AUXSTORE_VARSTRUCT_VAR(int,                                  lvl1a);
      // @TODO spans for all or just bare pointers and n_rdos only once ?
      xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<Identifier::value_type>     rdoList;
      xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<int>                        totList;
      xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<float>                      chargeList;
    };

    /// @}
};

inline
SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
PixelCluster_v1::rdoList() const {
   return s_rdoListAcc(*this);
}

inline
SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
PixelCluster_v1::totList() const {
   return s_totListAcc(*this);
}

inline
SG::ConstAccessor<SG::JaggedVecElt<float> >::element_type
PixelCluster_v1::chargeList() const {
   return s_chargeListAcc(*this);
}

}  // namespace xAOD
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::PixelCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif
