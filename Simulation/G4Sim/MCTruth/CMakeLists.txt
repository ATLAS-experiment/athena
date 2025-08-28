################################################################################
# Package: MCTruth
################################################################################

# Declare the package name:
atlas_subdir( MCTruth )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )

# Component(s) in the package:
atlas_add_library( MCTruth
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS MCTruth
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} 
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} HitManagement AtlasHepMCLib TruthUtils AthenaKernel GeneratorObjects
                   PRIVATE_LINK_LIBRARIES ISF_Event SimHelpers )
set_target_properties( MCTruth PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
