/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../RDOtoTracccCellConverterAlg.h"
#include "../PhaseIIRDOtoTracccCellConverterAlg.h"
#include "../TracccCellValidationAlg.h"
#include "../TracccMeasurementConverterAlg.h"
#include "../TracccSeedConverterAlg.h"
#include "../TracccTrackConverterAlg.h"
#include "../xAODToTracccMeasurementConverterAlg.h"
#include "../xAODToTracccSpacePointConverterAlg.h"
#include "../ClusterValidationAlg.h"
#include "../SeedValidationAlg.h"


DECLARE_COMPONENT( ActsTrk::RDOtoTracccCellConverterAlg )
DECLARE_COMPONENT( ActsTrk::PhaseIIRDOtoTracccCellConverterAlg )
DECLARE_COMPONENT( ActsTrk::TracccCellValidationAlg )
DECLARE_COMPONENT( ActsTrk::TracccMeasurementConverterAlg )
DECLARE_COMPONENT( ActsTrk::TracccSeedConverterAlg )
DECLARE_COMPONENT( ActsTrk::TracccTrackConverterAlg )
DECLARE_COMPONENT( ActsTrk::xAODToTracccMeasurementConverterAlg )
DECLARE_COMPONENT( ActsTrk::xAODToTracccSpacePointConverterAlg )
DECLARE_COMPONENT( ActsTrk::ClusterValidationAlg )
DECLARE_COMPONENT( ActsTrk::SeedValidationAlg )


#ifdef ACTSGPUEVENTCNV_WITH_CUDA_COMPONENTS

#include "src/cuda/TracccMeasurementDeviceConverterAlg.h"
DECLARE_COMPONENT( ActsTrk::TracccMeasurementDeviceConverterAlg )
#include "../cuda/TracccSeedDeviceConverterAlg.h"
DECLARE_COMPONENT( ActsTrk::TracccSeedDeviceConverterAlg )

#endif
