# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonG4SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )
find_package( GTest )

atlas_add_library( MuonG4SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS MuonG4SD
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} StoreGateLib GeoPrimitives GaudiKernel MuonSimEvent G4AtlasToolsLib MCTruth MuonIdHelpersLib)

# Component(s) in the package:
atlas_add_library( MuonG4SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} MuonG4SDLib StoreGateLib GeoPrimitives GaudiKernel MuonSimEvent G4AtlasToolsLib MCTruth )

# Turn on/off LTO for all libraries in the package.
set_target_properties(
   MuonG4SDLib
   MuonG4SD
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Helper function setting up the tests in the package:
function( _add_test name )
   atlas_add_test( ${name}
      SOURCES test/${name}.cxx
      INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} ${GTEST_LIBRARIES} TestTools MuonG4SDLib StoreGateLib GeoPrimitives GaudiKernel MuonSimEvent G4AtlasToolsLib MCTruth CxxUtils
      POST_EXEC_SCRIPT noerror.sh )
   set_target_properties( MuonG4SD_${name}
      PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
endfunction()

_add_test( CSCSensitiveDetector_gtest )
_add_test( CSCSensitiveDetectorCosmics_gtest )
_add_test( GenericMuonSensitiveDetector_gtest )
_add_test( MDTSensitiveDetector_gtest )
_add_test( MDTSensitiveDetectorCosmics_gtest )
_add_test( MicromegasSensitiveDetector_gtest )
_add_test( RPCSensitiveDetector_gtest )
_add_test( RPCSensitiveDetectorCosmics_gtest )
_add_test( TGCSensitiveDetector_gtest )
_add_test( TGCSensitiveDetectorCosmics_gtest )
_add_test( sTGCSensitiveDetector_gtest )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )
