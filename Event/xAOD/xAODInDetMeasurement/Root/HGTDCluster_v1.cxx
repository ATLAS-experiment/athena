/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODInDetMeasurement/versions/HGTDCluster_v1.h"

// rdoList
const SG::AuxElement::Accessor<SG::JaggedVecElt<Identifier::value_type> >
    xAOD::HGTDCluster_v1::s_rdoListAcc("rdoListjv");


void xAOD::HGTDCluster_v1::setRDOlist(std::vector<Identifier::value_type>&& rdoList) {
   s_rdoListAcc.set(*this,rdoList);
}
void xAOD::HGTDCluster_v1::setRDOlist(const std::vector<Identifier>& rdoList) {
    std::vector<Identifier::value_type> rdos(rdoList.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i] = rdoList[i].get_compact();
    }
    s_rdoListAcc.set(*this,rdos);
}

// totList
const SG::AuxElement::Accessor<SG::JaggedVecElt<int> >
    xAOD::HGTDCluster_v1::s_totListAcc("totListjv");

void xAOD::HGTDCluster_v1::setToTlist(const std::vector<int>& tots) {
   s_totListAcc.set(*this,tots);
}
