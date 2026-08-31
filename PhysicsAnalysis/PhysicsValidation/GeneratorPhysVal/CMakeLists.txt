# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Declare the package name:
atlas_subdir( GeneratorPhysVal )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( GeneratorPhysVal
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthenaBaseComps AthenaMonitoringLib AtlasHepMCLib GaudiKernel PathResolver StoreGateLib TrkValHistUtils TruthUtils xAODBase xAODEventInfo xAODJet xAODTruth )

atlas_install_python_modules( python/*py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*py POST_BUILD_CMD ${ATLAS_FLAKE8} )
