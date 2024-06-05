/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackMeasurement_v1.h"


namespace xAOD {
    const SG::AuxElement::Accessor<std::vector<double>> TrackMeasurement_v1::s_measAcc("meas");
    const SG::AuxElement::Accessor<std::vector<double>> TrackMeasurement_v1::s_covMatrixAcc("covMatrix");

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::vector<double>,
                                        meas,
                                        setMeas)

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::vector<double>,
                                        covMatrix,
                                        setCovMatrix)

    const xAOD::UncalibratedMeasurement *
    TrackMeasurement_v1::uncalibratedMeasurement() const {
       static const SG::ConstAccessor<const xAOD::UncalibratedMeasurement *> acc("uncalibratedMeasurement");
       return acc.isAvailable(*this) ?  acc(*this) : nullptr;
    }

    void
    TrackMeasurement_v1::setUncalibratedMeasurement(const xAOD::UncalibratedMeasurement *an_uncalibrated_measurement) {
       static const SG::Decorator<const xAOD::UncalibratedMeasurement *> decor("uncalibratedMeasurement");
       decor(*this) = an_uncalibrated_measurement;
    }

    AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackMeasurement_v1,
                                        std::uint64_t,
                                        projector,
                                        setProjector)

    const std::uint64_t* TrackMeasurement_v1::projectorPtr() const {
        static const ConstAccessor<std::uint64_t> acc("projector");
        return &(acc(*this));
    }
    std::uint64_t* TrackMeasurement_v1::projectorPtr() {
        static const Accessor<std::uint64_t> acc("projector");
         return &(acc(*this));
    }

    void TrackMeasurement_v1::resize(size_t sz) {
        s_measAcc(*this).resize(sz);
        s_covMatrixAcc(*this).resize(sz * sz);
    }

    size_t TrackMeasurement_v1::size() const {
      return s_measAcc(*this).size();
    }
}
