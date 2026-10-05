/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODInDetMeasurement/versions/PixelCluster_v1.h"
#include "xAODInDetMeasurement/ArrayFloat3.h"

static const SG::Accessor<xAOD::ArrayFloat3> globalPosAcc(
    "globalPosition");
const SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >
    xAOD::PixelCluster_v1::s_rdoListAcc("rdoList");

xAOD::ConstVectorMap<3> xAOD::PixelCluster_v1::globalPosition() const {
    const auto& values = globalPosAcc(*this);
    return ConstVectorMap<3>{values.data()};
}

xAOD::VectorMap<3> xAOD::PixelCluster_v1::globalPosition() {
    auto& values = globalPosAcc(*this);
    return VectorMap<3>{values.data()};
}


void xAOD::PixelCluster_v1::setRDOlist(std::vector<Identifier::value_type>&& rdoList) {
   s_rdoListAcc.set(*this,rdoList);
}
void xAOD::PixelCluster_v1::setRDOlist(std::span<Identifier::value_type> rdoList) {
   s_rdoListAcc.set(*this,rdoList);
}

//Custom setter for identifier inputs
void xAOD::PixelCluster_v1::setRDOlist(const std::vector<Identifier>& rdoList) {
    std::vector<Identifier::value_type> rdos(rdoList.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i] = rdoList[i].get_compact();
    }
    s_rdoListAcc.set(*this,rdos);
}

const SG::Accessor<SG::JaggedVecElt<int> >
    xAOD::PixelCluster_v1::s_totListAcc("totList");

void xAOD::PixelCluster_v1::setToTlist(const std::vector<int>& tots) {
   s_totListAcc.set(*this,tots);
}

void xAOD::PixelCluster_v1::setToTlist(std::span<int> tots) {
   s_totListAcc.set(*this,tots);
}
const SG::Accessor<SG::JaggedVecElt<float> >
    xAOD::PixelCluster_v1::s_chargeListAcc("chargeList");

void xAOD::PixelCluster_v1::setChargelist(const std::vector<float>& charges) {
   s_chargeListAcc.set(*this,charges);
}

void xAOD::PixelCluster_v1::setChargelist(std::span<float> charges) {
   s_chargeListAcc.set(*this,charges);
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, totalCharge,
				     setTotalCharge)

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, int, channelsInPhi)

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, int, channelsInEta)

void xAOD::PixelCluster_v1::setChannelsInPhiEta(int channelsInPhi,
                                                int channelsInEta) {
    static const SG::Accessor<int> chanPhiAcc("channelsInPhi");
    chanPhiAcc(*this) = channelsInPhi;
    static const SG::Accessor<int> chanEtaAcc("channelsInEta");
    chanEtaAcc(*this) = channelsInEta;
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, widthInEta,
                                     setWidthInEta)





AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, energyLoss,
                                     setEnergyLoss)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, int, lvl1a,
                                     setLVL1A)
