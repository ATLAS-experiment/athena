/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODInDetMeasurement/versions/HGTDCluster_v1.h"

// rdoList
static const SG::AuxElement::Accessor<std::vector<Identifier::value_type> >
    rdoListAcc("rdoList");


void xAOD::HGTDCluster_v1::setRDOlist(const std::vector<Identifier>& rdoList) {
    std::vector<Identifier::value_type> rdos(rdoList.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i] = rdoList[i].get_compact();
    }
    rdoListAcc(*this) = rdos;
}

const std::vector<Identifier> xAOD::HGTDCluster_v1::rdoList() const {
    const std::vector<Identifier::value_type>& values = rdoListAcc(*this);
    std::vector<Identifier> rdos(values.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i].set_literal(values[i]);
    }
    return rdos;
}


// totList
AUXSTORE_OBJECT_SETTER_AND_GETTER(xAOD::HGTDCluster_v1, std::vector<int>, totList,
				  setToTlist)
