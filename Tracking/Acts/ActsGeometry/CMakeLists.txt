# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometry )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson PluginGeoModel PluginRoot PluginDetray)
find_package( Boost )
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelKernel )
find_package( ROOT COMPONENTS Core Tree RIO )
find_package( TBB )
find_package( traccc )

# Component(s) in the package:
atlas_add_library( ActsGeometryLib
                   src/ActsDetectorElement.cxx
                   src/ActsLayerBuilder.cxx
                   src/ActsStrawLayerBuilder.cxx
                   src/ActsHGTDLayerBuilder.cxx                   
                   src/BeamPipeBlueprintNodeBuilder.cxx
                   src/HgtdBlueprintNodeBuilder.cxx
                   src/ItkBlueprintNodeBuilder.cxx
                   src/CaloBlueprintNodeBuilder.cxx
                   src/ContextUtility.cxx
                   src/util/*.cxx
                   PUBLIC_HEADERS ActsGeometry
                   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${TBB_LIBRARIES}
                   Acts::Core
                   ActsCalibBase
                   ActsGeometryInterfacesLib
                   ActsGeoUtils
                   ActsInteropLib
                   Acts::PluginJson
                   Acts::PluginGeoModel
                   Acts::PluginRoot
                   AthenaBaseComps
                   AthenaKernel
                   CaloDetDescrLib
                   CaloDetDescrUtils
                   GaudiKernel
                   GeoModelUtilities
                   GeoPrimitives
                   HGTD_Identifier
                   HGTD_ReadoutGeometry
                   Identifier
                   InDetIdentifier
                   InDetReadoutGeometry
                   MagFieldConditions
                   StoreGateLib
                   TRT_ReadoutGeometry
                   SCT_ReadoutGeometry
                   TrkSurfaces
                   xAODMeasurementBase
                   PRIVATE_LINK_LIBRARIES
                   TrkGeometry
                   BeamPipeGeoModelLib
                   )

atlas_add_component( ActsGeometry
                     src/WriteTrackingGeometry.cxx
                     src/ActsWriteTrackingGeometryTransforms.cxx
                     src/ExtrapolationTool.cxx
                     src/ActsCaloTrackingVolumeBuilder.cxx
                     src/ITkMaterialDecoratorTool.cxx
                     src/TrackingGeometrySvc.cxx
                     src/TrackingGeoAlignVisitor.cxx
                     src/GeometryEnvelopeTest.cxx
                     src/ActsVolumeIdToDetectorElementCollectionMappingAlg.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} ${ROOT_LIBRARIES}
                     Acts::PluginDetray
                     traccc::core
                     AthDeviceInterfacesLib
                     ActsGeometryInterfacesLib
                     ActsGeometryLib
                     ActsInteropLib
                     EventPrimitives
                     GeoPrimitives
                     AthenaBaseComps
                     AthenaKernel
                     CaloDetDescrLib
                     CxxUtils
                     EventInfo
                     GaudiKernel
                     StoreGateLib
                     TRT_ReadoutGeometry
                     TrkGeometry
                     TrkParameters
                     TrkSurfaces
                     Acts::PluginGeoModel
                     PathResolver
                     BeamPipeGeoModelLib
                    )

# Install files from the package:
atlas_install_joboptions( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if ( NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  # Disable this test in the debug build --- it takes too long to run.
  atlas_add_test( CombinedGen3TrackTest
                  PROPERTIES TIMEOUT 2000
                  PRIVATE_WORKING_DIRECTORY
                  SCRIPT ActsITkMuonCaloTest.py
                  POST_EXEC_SCRIPT noerror.sh)
  atlas_add_test( GeometryEnvelopeTest
                  PROPERTIES TIMEOUT 2000
                  PRIVATE_WORKING_DIRECTORY
                  SCRIPT ActsEnvelopeTest.py
                  POST_EXEC_SCRIPT noerror.sh)
endif()
