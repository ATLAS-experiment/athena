# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( G4HiveEx )

# External dependencies:
find_package( Geant4 )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( G4HiveEx
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} AthenaKernel AthenaBaseComps CxxUtils GeneratorObjects )
