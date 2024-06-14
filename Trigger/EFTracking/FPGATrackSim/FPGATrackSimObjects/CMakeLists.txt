# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimObjects )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist )

# Component(s) in the package:
atlas_add_root_dictionary( FPGATrackSimObjectsLib
   FPGATrackSimObjectsLibDictSource
   ROOT_HEADERS            FPGATrackSimObjects/*.h
   EXTERNAL_PACKAGES       ROOT
)

atlas_add_library( FPGATrackSimObjectsLib
   src/*.cxx FPGATrackSimObjects/*.h src/*.h ${FPGATrackSimObjectsLibDictSource}
   PUBLIC_HEADERS          FPGATrackSimObjects
   INCLUDE_DIRS            ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES          ${ROOT_LIBRARIES} AthContainers
)

# Tests in the package:
atlas_add_test(             FPGATrackSimRoad_test
    SOURCES                 test/FPGATrackSimRoad_test.cxx
    LINK_LIBRARIES          FPGATrackSimObjectsLib
)

atlas_add_test(             FPGATrackSimTrack_test
    SOURCES                 test/FPGATrackSimTrack_test.cxx
    LINK_LIBRARIES          FPGATrackSimObjectsLib
)

atlas_add_test(             FPGATrackSimLogicalEventInputHeader_test
    SOURCES                 test/FPGATrackSimLogicalEventInputHeader_test.cxx
    LINK_LIBRARIES          FPGATrackSimObjectsLib
)

atlas_add_test(             FPGATrackSimLogicalEventOutputHeader_test
    SOURCES                 test/FPGATrackSimLogicalEventOutputHeader_test.cxx
    LINK_LIBRARIES          FPGATrackSimObjectsLib
)
