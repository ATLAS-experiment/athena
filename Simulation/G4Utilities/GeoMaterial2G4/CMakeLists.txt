# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeoMaterial2G4 )

# External dependencies:
find_package( Geant4 )
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_library( GeoMaterial2G4
                   src/*.cxx
                   OBJECT
                   PUBLIC_HEADERS GeoMaterial2G4
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} GeoModelCore::GeoModelKernel AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES CxxUtils GeoModelUtilities )
set_target_properties( GeoMaterial2G4 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )
