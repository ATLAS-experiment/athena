# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LUCID_G4_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( LUCID_G4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib HitManagement LUCID_GeoModelLib LUCID_SimEvent MCTruth StoreGateLib )

atlas_add_library( LUCID_G4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES LUCID_G4_SDLib )

atlas_add_test( LUCID_SensitiveDetector_gtest
                SOURCES test/LUCID_SensitiveDetector_gtest.cxx
                LINK_LIBRARIES LUCID_G4_SDLib G4AtlasToolsLib MCTruth TestTools CxxUtils
                POST_EXEC_SCRIPT noerror.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   LUCID_G4_SDLib
   LUCID_G4_SD
   LUCID_G4_SD_LUCID_SensitiveDetector_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )
