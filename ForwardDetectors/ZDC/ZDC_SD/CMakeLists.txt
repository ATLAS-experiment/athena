# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZDC_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( ZDC_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib HitManagement StoreGateLib ZDC_SimEvent ZdcIdentifier CaloG4SimLib CaloSimEvent CaloIdentifier MCTruth )

atlas_add_library( ZDC_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES ZDC_SDLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )

# Tests in the package:
atlas_add_test( ZDC_FiberSD_gtest
                SOURCES test/ZDC_FiberSD_gtest.cxx
                LINK_LIBRARIES TestTools ZDC_SDLib G4AtlasToolsLib CxxUtils CaloG4SimLib
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( ZDC_G4CalibSD_gtest
                SOURCES test/ZDC_G4CalibSD_gtest.cxx
                LINK_LIBRARIES TestTools ZDC_SDLib G4AtlasToolsLib CxxUtils CaloG4SimLib
                POST_EXEC_SCRIPT noerror.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   ZDC_SDLib
   ZDC_SD
   ZDC_SD_ZDC_FiberSD_gtest
   ZDC_SD_ZDC_G4CalibSD_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
