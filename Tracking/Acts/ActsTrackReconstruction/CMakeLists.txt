# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrackReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson )
find_package( Boost )

atlas_add_component( ActsTrackReconstruction
                     src/*.cxx  src/detail/*.cxx
                     src/components/*.cxx
             INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
            LINK_LIBRARIES ${Boost_LIBRARIES}
               Acts::Core
               ActsEventLib
               ActsGeometryLib
               ActsGeometryInterfacesLib
               ActsCalibratorsLib
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
               MuonRecToolInterfacesR4
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
               InDetMeasurementUtilitiesLib)
             
atlas_add_test ( TrackFindingMeasurementsTest
  SOURCES test/TrackFindingMeasurementsTest.cxx src/detail/TrackFindingMeasurements.cxx
  LINK_LIBRARIES ActsEventLib xAODInDetMeasurement ActsCalibratorsLib
  POST_EXEC_SCRIPT noerror.sh )

atlas_add_test ( DuplicateSeedDetectorTest
  SOURCES test/DuplicateSeedDetectorTest.cxx
  LINK_LIBRARIES ActsEventLib xAODInDetMeasurement ActsCalibratorsLib
  POST_EXEC_SCRIPT noerror.sh)


