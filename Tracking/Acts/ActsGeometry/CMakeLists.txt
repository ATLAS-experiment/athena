# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometry )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson PluginGeoModel )
find_package( Boost )
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelKernel )
find_package( ROOT COMPONENTS Core Tree RIO )
find_package( TBB )

# Component(s) in the package:
atlas_add_library( ActsGeometryLib
                   src/ActsDetectorElement.cxx
                   src/ActsLayerBuilder.cxx
                   src/ActsStrawLayerBuilder.cxx
                   src/ActsHGTDLayerBuilder.cxx
                   src/ActsTrackingGeometrySvc.cxx
                   src/DetectorVolumeSvc.cxx
                   src/SimpleCylinderDetBuilderTool.cxx
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
                   PathResolver
                   TrkGeometry
                   BeamPipeGeoModelLib
                   )

atlas_add_component( ActsGeometry
                     src/ActsExtrapolationAlg.cxx
                     src/ActsWriteTrackingGeometry.cxx
                     src/ActsWriteTrackingGeometryTransforms.cxx
                     src/ExtrapolationTool.cxx
                     src/ActsMaterialMapping.cxx
                     src/ActsSurfaceMappingTool.cxx
                     src/ActsVolumeMappingTool.cxx
                     src/ActsMaterialStepConverterTool.cxx
                     src/ActsMaterialJsonWriterTool.cxx
                     src/ActsMaterialTrackWriterSvc.cxx
                     src/ActsTrackingGeometryTool.cxx
                     src/ActsPropStepRootWriterSvc.cxx
                     src/ActsCaloTrackingVolumeBuilder.cxx
                     src/ActsVolumeIdToDetectorElementCollectionMappingAlg.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} ${ROOT_LIBRARIES}
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
                    )

# Install files from the package:
atlas_install_joboptions( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_dictionary( ActsGeometryDict
                      ActsGeometry/ActsGeometryDict.h
                      ActsGeometry/selection.xml 
                      LINK_LIBRARIES ActsGeometryLib)
