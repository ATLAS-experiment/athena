/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODInDetMeasurement/versions/PixelCluster_v1.h"

static const SG::AuxElement::Accessor<std::array<float, 3> > globalPosAcc(
    "globalPosition");
static const SG::AuxElement::Accessor<std::vector<Identifier::value_type> >
    rdoListAcc("rdoList");

xAOD::ConstVectorMap<3> xAOD::PixelCluster_v1::globalPosition() const {
    const auto& values = globalPosAcc(*this);
    return ConstVectorMap<3>{values.data()};
}

xAOD::VectorMap<3> xAOD::PixelCluster_v1::globalPosition() {
    auto& values = globalPosAcc(*this);
    return VectorMap<3>{values.data()};
}

void xAOD::PixelCluster_v1::setRDOlist(const std::vector<Identifier>& rdoList) {
    std::vector<Identifier::value_type> rdos(rdoList.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i] = rdoList[i].get_compact();
    }
    rdoListAcc(*this) = std::move(rdos);
}

const std::vector<Identifier> xAOD::PixelCluster_v1::rdoList() const {
    const std::vector<Identifier::value_type>& values = rdoListAcc(*this);
    std::vector<Identifier> rdos(values.size());
    for (std::size_t i(0); i < rdos.size(); ++i) {
        rdos[i].set_literal(values[i]);
    }
    return rdos;
}

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, int, channelsInPhi)

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, int, channelsInEta)

void xAOD::PixelCluster_v1::setChannelsInPhiEta(int channelsInPhi,
                                                int channelsInEta) {
    static const SG::AuxElement::Accessor<int> chanPhiAcc("channelsInPhi");
    chanPhiAcc(*this) = channelsInPhi;
    static const SG::AuxElement::Accessor<int> chanEtaAcc("channelsInEta");
    chanEtaAcc(*this) = channelsInEta;
}

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, widthInEta,
                                     setWidthInEta)

void xAOD::PixelCluster_v1::setOmegas(float omegaX, float omegaY) {
    static const SG::AuxElement::Accessor<float> omegaXAcc("omegaX");
    omegaXAcc(*this) = omegaX;
    static const SG::AuxElement::Accessor<float> omegaYAcc("omegaY");
    omegaYAcc(*this) = omegaY;
}

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, float, omegaX)
AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, float, omegaY)

AUXSTORE_OBJECT_SETTER_AND_GETTER(xAOD::PixelCluster_v1, std::vector<int>, totList,
				  setToTlist)

AUXSTORE_OBJECT_MOVE(xAOD::PixelCluster_v1, std::vector<int>, totList, setToTlist)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, int, totalToT,
				     setTotalToT)

AUXSTORE_OBJECT_SETTER_AND_GETTER(xAOD::PixelCluster_v1, std::vector<float>, chargeList,
				  setChargelist)

AUXSTORE_OBJECT_MOVE(xAOD::PixelCluster_v1, std::vector<float>, chargeList, setChargelist)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, totalCharge,
				     setTotalCharge)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, float, energyLoss,
                                     setEnergyLoss)

AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(xAOD::PixelCluster_v1, char, bool, isSplit)
AUXSTORE_PRIMITIVE_SETTER_WITH_CAST(xAOD::PixelCluster_v1, char, bool, isSplit,
                                    setIsSplit)

void xAOD::PixelCluster_v1::setSplitProbabilities(float prob1, float prob2) {
    static const SG::AuxElement::Accessor<float> prob1Acc("splitProbability1");
    prob1Acc(*this) = prob1;
    static const SG::AuxElement::Accessor<float> prob2Acc("splitProbability2");
    prob2Acc(*this) = prob2;
}

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, float, splitProbability1)

AUXSTORE_PRIMITIVE_GETTER(xAOD::PixelCluster_v1, float, splitProbability2)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(xAOD::PixelCluster_v1, int, lvl1a,
                                     setLVL1A)
