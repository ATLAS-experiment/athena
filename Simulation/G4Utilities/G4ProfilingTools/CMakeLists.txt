# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( G4ProfilingTools )

# External dependencies:
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Hist RIO )

# Component(s) in the package:
atlas_add_library( G4ProfilingToolsLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   LINK_LIBRARIES G4AtlasToolsLib
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${GEANT4_LIBRARIES} AthenaBaseComps GaudiKernel G4AtlasToolsLib )
set_target_properties( G4ProfilingToolsLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_library( G4ProfilingTools
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES G4ProfilingToolsLib )
set_target_properties( G4ProfilingTools PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
