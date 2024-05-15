/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENTAUXCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENTAUXCONTAINER_V1_H


#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"
namespace xAOD {
 class TrackMeasurementAuxContainer_v1 : public AuxContainerBase {
    public:
        TrackMeasurementAuxContainer_v1();
        // we use vector instead of array even though the size is fixed
        // this saves on generating ROOT dictionaries for all array dimensions
        typedef std::vector<double> Storage;
        std::vector<Storage> meas;
        std::vector<Storage> covMatrix;
        std::vector<const xAOD::UncalibratedMeasurement *> uncalibratedMeasurement;
        std::vector<std::uint64_t> projector;
    };
}

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TrackMeasurementAuxContainer_v1, xAOD::AuxContainerBase);


#endif
