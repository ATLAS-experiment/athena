################################################################################
# Package: MCTruthAlgs
################################################################################

# Declare the package name:
atlas_subdir( MCTruthAlgs )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( MCTruthAlgs
                             src/*.cxx src/components/*.cxx
                             INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                             LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps GaudiKernel TrackRecordLib )
