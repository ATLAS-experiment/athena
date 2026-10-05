/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackJacobian_v1.h"

namespace xAOD {
    static const SG::Accessor<std::vector<double>> jacAcc("jac");
    TrackJacobian_v1::MatrixMap TrackJacobian_v1::jacEigen() {
        return MatrixMap{jacAcc(*this).data()};
    }

    TrackJacobian_v1::ConstMatrixMap TrackJacobian_v1::jacEigen() const {
        return ConstMatrixMap{jacAcc(*this).data()};
    }

    void TrackJacobian_v1::resize(size_t sz) {
        jacAcc(*this).resize(sz);
    }
}