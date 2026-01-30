# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Epos4_i )

# External dependencies:
find_package( CLHEP )
find_package( EPOS4 )

# Remove the --as-needed linker flags:
atlas_disable_as_needed()

# Component(s) in the package:
atlas_add_library( Epos4_iLib
                   src/Epos4.cxx
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES  GeneratorModulesLib
                   PRIVATE_LINK_LIBRARIES AtlasHepMCLib AtlasHepMCfioLib ${CLHEP_LIBRARIES} GaudiKernel TruthUtils ${EPOS4_LIBRARIES})

atlas_add_component( Epos4_i
                     src/components/*.cxx
                     LINK_LIBRARIES Epos4_iLib )

# Install files from the package:
atlas_install_joboptions( share/common/*.py )

# Set up the runtime environment for the package.
set( Epos4Environment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of Epos4Environment.cmake" )
find_package( Epos4Environment )
