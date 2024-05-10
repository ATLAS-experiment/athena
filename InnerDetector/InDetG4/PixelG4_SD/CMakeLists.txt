# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelG4_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelRead GeoModelDBManager )

# Component(s) in the package:
atlas_add_library( PixelG4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} ${GEOMODEL_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib GeoModelInterfaces GeoPrimitives)

atlas_add_library( PixelG4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES PixelG4_SDLib )

# Test(s) in the package:
atlas_add_test( PixelG4_SDToolConfig_test
                SCRIPT test/PixelG4_SDToolConfig_test.py
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( PixelSensorSD_gtest
                SOURCES
                test/PixelSensorSD_gtest.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib TestTools PixelG4_SDLib
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( PixelSensorGmxSD_gtest
                SOURCES
                test/PixelSensorGmxSD_gtest.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} G4AtlasToolsLib InDetSimEvent MCTruth StoreGateLib TestTools PixelG4_SDLib
                POST_EXEC_SCRIPT nopost.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   PixelG4_SDLib
   PixelG4_SD
   PixelG4_SD_PixelSensorSD_gtest
   PixelG4_SD_PixelSensorGmxSD_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )
