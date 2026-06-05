# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ForwardTransportSvc )

# External dependencies:
find_package( Geant4 )
find_package( ROOT COMPONENTS Tree )

atlas_add_library( ForwardTransportSvcLib
                   ForwardTransportSvc/*.h
                   INTERFACE
                   PUBLIC_HEADERS ForwardTransportSvc
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} AtlasHepMCLib ForwardTracker GaudiKernel )
set_target_properties( ForwardTransportSvcLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Component(s) in the package:
atlas_add_library( ForwardTransportSvc
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps ForwardTracker ForwardTransportSvcLib GaudiKernel GeneratorObjects )
set_target_properties( ForwardTransportSvc PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

