################################################################################
# Package: SimHelpers
################################################################################

# Declare the package name:
atlas_subdir( SimHelpers )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( SimHelpers
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS SimHelpers
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} GaudiKernel StoreGateLib )

set_target_properties( SimHelpers PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
