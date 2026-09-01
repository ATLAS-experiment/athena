# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometry )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson PluginGeoModel PluginRoot OPTIONAL_COMPONENTS PluginDetray)
find_package( Boost )
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelKernel )
find_package( ROOT COMPONENTS Core Tree RIO )
find_package( TBB )

# The conversion of the Gen3 Acts::TrackingGeometry into a Detray geometry is
# only available if ACTS was built with the Detray plugin.
set( _actsGeometryDetrayDefinitions )
set( _actsGeometryDetrayLibraries )
if( TARGET Acts::PluginDetray )
  list( APPEND _actsGeometryDetrayDefinitions ACTSGEOMETRY_HAVE_DETRAY )
  list( APPEND _actsGeometryDetrayLibraries Acts::PluginDetray )
endif()

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
                   src/util/*.cxx
                   PUBLIC_HEADERS ActsGeometry
                   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${TBB_LIBRARIES}
                   Acts::Core
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
                     src/ActsVolumeIdToDetectorElementCollectionMappingAlg.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                     DEFINITIONS ${_actsGeometryDetrayDefinitions}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} ${ROOT_LIBRARIES}
                     ${_actsGeometryDetrayLibraries}
                     ActsGeometryInterfacesLib
                     ActsGeometryLib
                     ActsInteropLib
                     ActsEventLib
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
                     detray::io
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
endif()
