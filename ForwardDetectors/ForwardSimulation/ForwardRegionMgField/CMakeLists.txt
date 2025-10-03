# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ForwardRegionMgField )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( ForwardRegionMgField
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} GaudiKernel AthenaBaseComps G4AtlasInterfaces PathResolver ForwardRegionGeoModelLib )
set_target_properties( ForwardRegionMgField PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_data( share/*.dat )
