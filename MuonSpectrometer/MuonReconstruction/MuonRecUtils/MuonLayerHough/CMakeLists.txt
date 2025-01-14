################################################################################
# Package: MuonLayerHough
################################################################################

# Declare the package name:
atlas_subdir( MuonLayerHough )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist )

# Component(s) in the package:
atlas_add_library( MuonLayerHough
                   src/*.cxx
                   PUBLIC_HEADERS MuonLayerHough
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES CxxUtils MuonStationIndexLib GaudiKernel AthenaKernel
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} GeoPrimitives )

