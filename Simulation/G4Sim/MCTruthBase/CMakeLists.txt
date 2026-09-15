# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MCTruthBase )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( MCTruthBaseLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} AthenaBaseComps GaudiKernel G4AtlasToolsLib ISF_InterfacesLib TrackRecordLib
                   PRIVATE_LINK_LIBRARIES G4AtlasInterfaces MCTruth
                   AtlasDetDescr ISF_Geant4Event
                 )
set_target_properties( MCTruthBaseLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_library(MCTruthBase
                  src/components/*.cxx
                  OBJECT
                  NO_PUBLIC_HEADERS
                  PRIVATE_LINK_LIBRARIES MCTruthBaseLib)
set_target_properties( MCTruthBase PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
