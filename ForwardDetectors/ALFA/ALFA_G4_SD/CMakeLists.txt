# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ALFA_G4_SD )

# External dependencies:
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( ALFA_G4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} ALFA_SimEv G4AtlasToolsLib StoreGateLib )

atlas_add_library( ALFA_G4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES ALFA_G4_SDLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )

# Test(s) in the package:
atlas_add_test( ALFA_SensitiveDetector_gtest
                SOURCES test/ALFA_SensitiveDetector_gtest.cxx
                LINK_LIBRARIES ALFA_G4_SDLib G4AtlasToolsLib TestTools CxxUtils
                POST_EXEC_SCRIPT noerror.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   ALFA_G4_SDLib
   ALFA_G4_SD
   ALFA_G4_SD_ALFA_SensitiveDetector_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
