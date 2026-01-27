# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsDataPreparation)

# External dependencies:
find_package(Acts COMPONENTS Core)

atlas_add_component( ActsDataPreparation
   src/*.h src/*.cxx
   src/components/*.cxx
   LINK_LIBRARIES
   Acts::Core
   ActsCalibratorsLib
   ActsGeometryLib
   ActsGeometryInterfacesLib
   ActsToolInterfacesLib
   AthenaBaseComps
   AthenaMonitoringKernelLib
   BeamSpotConditionsData
   GaudiKernel
   InDetConditionsSummaryService
   InDetCondTools
   InDetIdentifier
   InDetRawData
   InDetReadoutGeometry
   IRegionSelector
   PixelReadoutGeometryLib
   ReadoutGeometryBase
   SCT_ReadoutGeometry
   SiClusterizationToolLib
   SiSpacePointFormationLib
   StoreGateLib
   TrigSteeringEvent
   TrkSurfaces
   xAODInDetMeasurement
   xAODMeasurementBase
   HGTD_RawData
   HGTD_CalibrationLib
   ViewAlgsLib InDetIdentifier
)

# Unit tests
atlas_add_test( ActsRegionalDataPrep_CompositeRoI
  SCRIPT test/runCompositeRoICode.py
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( ActsRegionalDataPrep_FullScanRoI
  SCRIPT test/runFullScanRoICode.py
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( ActsRegionalDataPrep_SingleRoI
  SCRIPT test/runSingleRoICode.py
  POST_EXEC_SCRIPT nopost.sh )
