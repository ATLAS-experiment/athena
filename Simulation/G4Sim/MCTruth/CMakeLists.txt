################################################################################
# Package: MCTruth
################################################################################

# Declare the package name:
atlas_subdir( MCTruth )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( MCTruth
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS MCTruth
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} HitManagement AtlasHepMCLib TruthUtils AthenaKernel GeneratorObjects
                   PRIVATE_LINK_LIBRARIES ISF_Event SimHelpers )
set_target_properties( MCTruth PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
