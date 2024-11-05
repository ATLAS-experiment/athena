# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimObjects )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist )

# Replaces older "ROOT dictionary" (i.e. CINT) with "reflex" dictionary.
# See https://its.cern.ch/jira/browse/ATLINFR-5511 for some discussion on this;
# CINT dictionaries can cause problems for the Athena buildsystem.
atlas_add_dictionary( FPGATrackSimObjectsDict
   FPGATrackSimObjects/FPGATrackSimObjectsDict.h
   FPGATrackSimObjects/selection.xml
   LINK_LIBRARIES FPGATrackSimObjectsLib
)

atlas_add_library( FPGATrackSimObjectsLib
   src/*.cxx FPGATrackSimObjects/*.h src/*.h 
   PUBLIC_HEADERS          FPGATrackSimObjects
   INCLUDE_DIRS            ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES          ${ROOT_LIBRARIES} AthContainers GeneratorObjects
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
