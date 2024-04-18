# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimMaps )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( Boost )
find_package( lwtnn )

# Component(s) in the package:
atlas_add_library(
    FPGATrackSimMapsLib      src/*.cxx FPGATrackSimMaps/*.h
    PUBLIC_HEADERS      FPGATrackSimMaps
    INCLUDE_DIRS        ${LWTNN_INCLUDE_DIRS}
    LINK_LIBRARIES      ${LWTNN_LIBRARIES} GaudiKernel FPGATrackSimConfToolsLib FPGATrackSimObjectsLib
    PRIVATE_INCLUDE_DIRS   ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
    PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} AsgMessagingLib AthenaBaseComps PathResolver
)

atlas_add_component(
    FPGATrackSimMaps         src/components/*.cxx
    LINK_LIBRARIES      FPGATrackSimMapsLib FPGATrackSimConfToolsLib
)

# Install files from the package:
atlas_install_python_modules( python/*.py )

# Tests in the package:
atlas_add_test(         FPGATrackSimPlaneMap
    SOURCES             test/FPGATrackSimPlaneMap_test.cxx
    LINK_LIBRARIES      AthenaKernel FPGATrackSimMapsLib TestTools
    PROPERTIES
        TIMEOUT         120
)

atlas_add_test(         FPGATrackSimRegionMap
    SOURCES             test/FPGATrackSimRegionMap_test.cxx
    LINK_LIBRARIES      AthenaKernel FPGATrackSimMapsLib TestTools
    PROPERTIES
        TIMEOUT         120
)


