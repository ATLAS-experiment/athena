# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODInDetMeasurementAthenaPool )

# Component(s) in the package.
atlas_add_poolcnv_library( xAODInDetMeasurementAthenaPoolPoolCnv
   FILES xAODInDetMeasurement/PixelClusterContainer.h
   xAODInDetMeasurement/PixelClusterAuxContainer.h
   xAODInDetMeasurement/StripClusterContainer.h
   xAODInDetMeasurement/StripClusterAuxContainer.h
   xAODInDetMeasurement/HGTDClusterContainer.h
   xAODInDetMeasurement/HGTDClusterAuxContainer.h
   xAODInDetMeasurement/SpacePointContainer.h
   xAODInDetMeasurement/SpacePointAuxContainer.h
   TYPES_WITH_NAMESPACE xAOD::PixelClusterContainer xAOD::PixelClusterAuxContainer xAOD::StripClusterContainer xAOD::StripClusterAuxContainer
   xAOD::HGTDClusterContainer xAOD::HGTDClusterAuxContainer
   xAOD::SpacePointContainer xAOD::SpacePointAuxContainer
   CNV_PFX xAOD
   LINK_LIBRARIES AthContainers AthenaKernel AthenaPoolCnvSvcLib
   AthenaPoolUtilities xAODInDetMeasurement )
