# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonSensitiveDetectorsR4 )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( MuonSensitiveDetectorsR4Lib
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS MuonSensitiveDetectorsR4
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES GeoPrimitives ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} StoreGateLib
                                  GaudiKernel xAODMuonSimHit G4AtlasToolsLib MCTruth MuonIdHelpersLib MuonReadoutGeometryR4)

atlas_add_library( MuonSensitiveDetectorsR4
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} MuonSensitiveDetectorsR4Lib
                                          StoreGateLib GeoPrimitives GaudiKernel xAODMuonSimHit G4AtlasToolsLib MCTruth )

# Turn on/off LTO for all libraries in the package.
set_target_properties(
   MuonSensitiveDetectorsR4Lib
   MuonSensitiveDetectorsR4
   PROPERTIES
   INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
