# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsMonitoring )

# External dependencies
find_package( Acts COMPONENTS Core )
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree TreePlayer )

# Component(s) in the package:
atlas_add_component( ActsMonitoring
   src/*.h src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} Acts::Core ActsCalibratorsLib ActsEventLib ActsGeometryInterfacesLib ActsGeometryLib
   ActsInteropLib ActsToolInterfacesLib ActsTruthLib AsgDataHandlesLib AthAllocators AthContainers AthenaBaseComps
   AthenaKernel AthenaMonitoringKernelLib AthenaMonitoringLib BeamSpotConditionsData CxxUtils GaudiKernel GeoPrimitives
   HGTD_Identifier HGTD_ReadoutGeometry Identifier InDetCondTools InDetIdentifier InDetMeasurementUtilitiesLib InDetPrepRawData
   InDetRawData InDetReadoutGeometry MagFieldConditions MagFieldElements MuonTesterTreeLib PixelReadoutGeometryLib
   ReadoutGeometryBase SCT_ReadoutGeometry SGTools StoreGateLib TrkSurfaces TrkTruthData TrkValHistUtils TruthUtils
   xAODEventInfo xAODInDetMeasurement xAODMeasurementBase xAODTracking MuonVisualizationHelpersR4 )

atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_executable( runActsGeoComparison
   util/runActsGeoComparison.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver Acts::Core ActsGeometryInterfacesLib GeoPrimitives Identifier )
