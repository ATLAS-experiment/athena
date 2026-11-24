# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FatrasToolsID )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Component(s) in the package:
atlas_add_component( ISF_FatrasToolsID
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel AthContainers CxxUtils Identifier GaudiKernel GeoPrimitives InDetSimEvent ISF_FatrasInterfaces TrkParameters StoreGateLib AtlasDetDescr IdDictDetDescr TRT_ConditionsServicesLib InDetIdentifier ReadoutGeometryBase InDetConditionsSummaryService InDetReadoutGeometry TRT_ReadoutGeometry InDetPrepRawData InDetRIO_OnTrack SiClusterizationToolLib ISF_Event ISF_InterfacesLib TrkDetElementBase TrkSurfaces TrkEventPrimitives TrkMeasurementBase TrkRIO_OnTrack TrkTrack TrkExUtils TrkToolInterfaces )
