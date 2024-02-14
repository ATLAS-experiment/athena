# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RHadrons )

if( NOT GENERATIONBASE )

    # External dependencies:
    find_package( CLHEP )
    find_package( Geant4 )

    # Component(s) in the package:
    atlas_add_library( RHadrons
                       src/*.cxx
                       src/components/*.cxx
                       OBJECT
                       NO_PUBLIC_HEADERS
                       PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                       PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} GaudiKernel AthenaBaseComps CxxUtils G4AtlasInterfaces G4AtlasToolsLib G4ExternalDecay SimHelpers TruthUtils)
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
