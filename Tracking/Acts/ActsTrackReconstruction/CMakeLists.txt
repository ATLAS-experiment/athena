# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrackReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson )
find_package( Boost )

atlas_add_component( ActsTrackReconstruction
                     src/*.h src/*.cxx src/detail/*.h src/detail/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
		       ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES
		       ${Boost_LIBRARIES}
                       ActsCore
		       ActsEventLib
		       ActsEventCnvLib
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
		       ActsInteropLib
		       ActsToolInterfacesLib
		       AthenaBaseComps
		       AthenaMonitoringKernelLib
		       BeamSpotConditionsData
		       GaudiKernel
		       GeoPrimitives
		       HGTD_PrepRawData
		       HGTD_RIO_OnTrack
		       Identifier
		       InDetIdentifier
		       InDetPrepRawData
		       InDetRIO_OnTrack
		       InDetReadoutGeometry
		       InDetRecToolInterfaces
		       MagFieldConditions
		       MagFieldElements
		       PixelConditionsData
		       StoreGateLib
		       TRT_ReadoutGeometry
		       TrkEventPrimitives
		       TrkFitterInterfaces
		       TrkMeasurementBase
		       TrkParameters
		       TrkPrepRawData
		       TrkRIO_OnTrack
		       TrkSurfaces
		       TrkToolInterfaces
		       TrkTrack
		       TrkTrackSummary
		       TrkTruthData
		       TruthUtils
		       xAODEventInfo
		       xAODInDetMeasurement
		       xAODMeasurementBase
		       xAODTracking
		       PathResolver
                     )
		     
atlas_add_test ( TrackFindingMeasurementsTest
  SOURCES test/TrackFindingMeasurementsTest.cxx
  LINK_LIBRARIES ActsEventLib xAODInDetMeasurement
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test ( DuplicateSeedDetectorTest
  SOURCES test/DuplicateSeedDetectorTest.cxx
  LINK_LIBRARIES ActsEventLib ActsEventCnvLib xAODInDetMeasurement
  POST_EXEC_SCRIPT nopost.sh)

atlas_add_test ( MeasurementCalibratorBaseTest
  SOURCES test/MeasurementCalibratorBaseTest.cxx
  LINK_LIBRARIES ActsCore ActsEventLib xAODInDetMeasurement ActsGeometryInterfacesLib
  POST_EXEC_SCRIPT nopost.sh )

atlas_add_test ( TrkMeasurementCalibratorTest
  SOURCES test/TrkMeasurementCalibratorTest.cxx
  LINK_LIBRARIES ActsCore ActsEventCnvLib TestTools AthenaBaseComps
  DEPENDS MeasurementCalibratorBaseTest
  POST_EXEC_SCRIPT nopost.sh )
