################################################################################
# Package: G4AtlasInterfaces
################################################################################

# Declare the package name:
atlas_subdir( G4AtlasInterfaces )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( G4AtlasInterfaces
                   PUBLIC_HEADERS G4AtlasInterfaces
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} GaudiKernel )

set_target_properties( G4AtlasInterfaces PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
