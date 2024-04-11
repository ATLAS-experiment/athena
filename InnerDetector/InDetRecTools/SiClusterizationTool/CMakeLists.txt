# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SiClusterizationTool )

# External dependencies:
find_package( lwtnn )
find_package( Eigen )
find_package( CLHEP )
find_package( ROOT COMPONENTS Core MathCore Hist )
find_package( COOL COMPONENTS CoolKernel CoolApplication )
find_package( Boost )

# Component(s) in the package:
atlas_add_library( SiClusterizationToolLib
   SiClusterizationTool/*.h src/*.cxx
   PUBLIC_HEADERS SiClusterizationTool
   INCLUDE_DIRS PixelGeoModelLib ${CLHEP_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   DEFINITIONS ${CLHEP_DEFINITIONS}
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${COOL_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${EIGEN_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AthAllocators
   BeamSpotConditionsData EventPrimitives GaudiKernel GeoPrimitives Identifier InDetCondTools InDetConditionsSummaryService 
   InDetIdentifier InDetPrepRawData InDetRawData InDetReadoutGeometry SCT_ReadoutGeometry InDetRecToolInterfaces
   InDetSimData PixelConditionsData PixelReadoutGeometryLib PoolSvcLib StoreGateLib TrkNeuralNetworkUtilsLib TrkParameters LwtnnUtils xAODInDetMeasurement
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${COOL_LIBRARIES} ${LWTNN_LIBRARIES} AthenaPoolUtilities AtlasDetDescr AtlasHepMCLib FileCatalog ReadoutGeometryBase TrkEventPrimitives TrkSurfaces VxVertex CxxUtils xAODCore )

atlas_add_component( SiClusterizationTool
   src/components/*.cxx
   LINK_LIBRARIES SiClusterizationToolLib )

# These files can be added by the user for testing in Grid environments,
# in which case un-comment this line and re-cmake
# atlas_install_data( share/*.db )
