# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BCM_G4_SD )

# External dependencies:
find_package( AtlasGeant4Utilities )
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( BCM_G4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} AthenaKernel G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib )

atlas_add_library( BCM_G4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES BCM_G4_SDLib )

# Test(s) in the package:
atlas_add_test( BCM_G4_SDToolConfig_test
                SCRIPT test/BCM_G4_SDToolConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( BCMSensorSD_gtest
                SOURCES
                test/BCMSensorSD_gtest.cxx
                INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib TestTools BCM_G4_SDLib
                POST_EXEC_SCRIPT noerror.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   BCM_G4_SDLib
   BCM_G4_SD
   BCM_G4_SD_BCMSensorSD_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )


# Turn on/off CUDA device symbol resolution for tests (Geant4 links to VecGeom, which is CUDA-enabled)
atlas_configure_g4_targets( TARGETS BCM_G4_SD_BCMSensorSD_gtest )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )
