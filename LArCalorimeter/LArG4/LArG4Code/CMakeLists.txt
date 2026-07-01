# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArG4Code )

# External dependencies:
find_package( AtlasGeant4Utilities )
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( LArG4Code
                   src/*.cc
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS LArG4Code
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   PRIVATE_DEFINITIONS LARG4NOROOT
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} AtlasDetDescr CaloSimEvent LArGeoCode LArSimEvent CaloG4SimLib G4AtlasToolsLib StoreGateLib GaudiKernel
                   PRIVATE_LINK_LIBRARIES CaloIdentifier AthenaKernel CxxUtils MCTruth )

atlas_add_dictionary( LArG4CodeEnums
                      LArG4Code/LArG4EnumDefs.h
                      LArG4Code/selectionEnums.xml )

atlas_add_test( LArG4CalibSD_gtest
                SOURCES test/LArG4CalibSD_gtest.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                DEFINITIONS ${CLHEP_DEFINITIONS}
                LINK_LIBRARIES TestTools LArG4Code ${GTEST_LIBRARIES} ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} AtlasDetDescr CaloSimEvent LArGeoCode LArSimEvent CaloG4SimLib G4AtlasToolsLib StoreGateLib GaudiKernel CaloIdentifier AthenaKernel CxxUtils MCTruth
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( LArG4SimpleSD_gtest
                SOURCES test/LArG4SimpleSD_gtest.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                DEFINITIONS ${CLHEP_DEFINITIONS}
                LINK_LIBRARIES TestTools LArG4Code ${GTEST_LIBRARIES} ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} AtlasDetDescr CaloSimEvent LArGeoCode LArSimEvent CaloG4SimLib G4AtlasToolsLib StoreGateLib GaudiKernel CaloIdentifier AthenaKernel CxxUtils MCTruth
                POST_EXEC_SCRIPT noerror.sh )

# Turn on/off LTO for all targets in the package.
set_target_properties(
   LArG4Code
   LArG4Code_LArG4CalibSD_gtest
   LArG4Code_LArG4SimpleSD_gtest
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Turn on/off CUDA device symbol resolution for tests (Geant4 links to VecGeom, which is CUDA-enabled)
atlas_configure_g4_targets( TARGETS
   LArG4Code_LArG4CalibSD_gtest
   LArG4Code_LArG4SimpleSD_gtest )

atlas_install_joboptions( share/optionForTest.txt )
