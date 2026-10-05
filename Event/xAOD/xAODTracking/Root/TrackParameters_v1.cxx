/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackParameters_v1.h"


namespace xAOD {
    static const SG::Accessor<std::vector<double>> paramsAcc("params");

    TrackParameters_v1::VectorMap TrackParameters_v1::paramsEigen() {
        return VectorMap{paramsAcc(*this).data()};
    }

    TrackParameters_v1::ConstVectorMap TrackParameters_v1::paramsEigen() const {
        return ConstVectorMap{paramsAcc(*this).data()};
    }

    static const SG::Accessor<std::vector<double>> covMatrixAcc("covMatrix");

    TrackParameters_v1::MatrixMap TrackParameters_v1::covMatrixEigen() {
        return MatrixMap{covMatrixAcc(*this).data()};
    }
    TrackParameters_v1::ConstMatrixMap TrackParameters_v1::covMatrixEigen() const {
        return ConstMatrixMap{covMatrixAcc(*this).data()};
    }

    void TrackParameters_v1::resize(size_t sz) {
        paramsAcc(*this).resize(sz);
        covMatrixAcc(*this).resize(sz * sz);
    }
}