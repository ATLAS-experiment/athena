################################################################################
# Package: MuonSensitiveDetectors
################################################################################

# Declare the package name:
atlas_subdir( MuonSensitiveDetectorsR4 )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )
find_package( GTest )


atlas_add_library( MuonSensitiveDetectorsR4Lib
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS MuonSensitiveDetectorsR4
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES GeoPrimitives ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} StoreGateLib  
                                  GaudiKernel xAODMuonSimHit G4AtlasToolsLib MCTruth MuonIdHelpersLib MuonReadoutGeometryR4)

# Component(s) in the package:
atlas_add_component( MuonSensitiveDetectorsR4
                     src/components/*.cxx                  
                     PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} MuonSensitiveDetectorsR4Lib 
                                          StoreGateLib GeoPrimitives GaudiKernel xAODMuonSimHit G4AtlasToolsLib MCTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

