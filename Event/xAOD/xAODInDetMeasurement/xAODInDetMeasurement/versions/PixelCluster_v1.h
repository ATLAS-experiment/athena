/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H
#define XAODINDETMEASUREMENT_VERSION_PIXELCLUSTER_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "AthContainers/JaggedVecAccessor.h"
#include "AthContainers/JaggedVecUtils.h"
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
         : xAOD::VariableStruct(cont)
      {
         if (n_cluster_rdos>0) {
            auto *store = cont.getStore();
            assert(store);
            SG::setJaggedVectorData(cont,*store,s_rdoListAcc, n_cluster_rdos, rdoList, rdoListPayload);
            SG::setJaggedVectorData(cont,*store,s_totListAcc, n_cluster_rdos, totList, totListPayload);
            SG::setJaggedVectorData(cont,*store,s_chargeListAcc, n_cluster_rdos, chargeList, chargeListPayload);
         }
      }

      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIdentType,              identifier);
      AUXSTORE_VARSTRUCT_VAR(xAOD::DetectorIDHashType,             identifierHash);
      AUXSTORE_VARSTRUCT_VAR(xAOD::PosAccessor<2>::element_type,   localPositionDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::CovAccessor<2>::element_type,   localCovarianceDim2);
      AUXSTORE_VARSTRUCT_VAR(xAOD::PosAccessor<3>::element_type,   globalPosition);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInPhi);
      AUXSTORE_VARSTRUCT_VAR(int,                                  channelsInEta);
      AUXSTORE_VARSTRUCT_VAR(float,                                widthInEta);
      AUXSTORE_VARSTRUCT_VAR(int,                                  lvl1a);
      // @TODO spans for all or just bare pointers and n_rdos only once ?
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Elt_span     rdoList;
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Payload_span rdoListPayload;
      SG::Accessor<SG::JaggedVecElt<int> >::Elt_span                        totList;
      SG::Accessor<SG::JaggedVecElt<int> >::Payload_span                    totListPayload;
      SG::Accessor<SG::JaggedVecElt<float> >::Elt_span                      chargeList;
      SG::Accessor<SG::JaggedVecElt<float> >::Payload_span                  chargeListPayload;
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
