/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "AthContainers/JaggedVecAccessor.h"

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
    SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
    rdoList() const;



    /// Returns the list of Time Over Threshold of the channels building the cluster
    SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
    totList() const;


    /// @name Functions to set HGTD cluster properties
    /// @{

    /// Sets the list of identifiers of the channels building the cluster
    void setRDOlist(std::vector<Identifier::value_type>&& rdoList);
    /// Sets the list of identifiers of the channels building the cluster
    /// This will first create a vector of Identifier values and then set the
    /// xAOD object  properties.
    void setRDOlist(const std::vector<Identifier>& rdolist);

    /// Sets the list of ToT of the channels building the cluster
    void setToTlist(const std::vector<int>& tots);

    static const SG::AuxElement::Accessor<SG::JaggedVecElt<Identifier::value_type> > rdoListAcc() { return s_rdoListAcc; }
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<int> > totListAcc() { return s_totListAcc; }
    /// @}
protected:
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<Identifier::value_type> > s_rdoListAcc;
    static const SG::AuxElement::Accessor<SG::JaggedVecElt<int> > s_totListAcc;
public:

};

inline
SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
HGTDCluster_v1::rdoList() const {
   return s_rdoListAcc(*this);
}

inline
SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
HGTDCluster_v1::totList() const {
   return s_totListAcc(*this);
}

} // namespace xAOD
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::HGTDCluster_v1, xAOD::UncalibratedMeasurement_v1);
#endif // XAODHGTDCluster_v1_H
