# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FlowAfterburner )

# External dependencies:
find_package( CLHEP )
find_package( GSL )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FlowAfterburnerLib
                   src/*.cxx
                   PUBLIC_HEADERS FlowAfterburner
                   INCLUDE_DIRS ${GSL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS 
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${GSL_LIBRARIES} ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel AtlasHepMCLib AtlasHepMCsearchLib GaudiKernel GeneratorObjects
                   PRIVATE_LINK_LIBRARIES CxxUtils StoreGateLib TruthUtils )

atlas_add_component( FlowAfterburner
                     src/components/*.cxx
                     PRIVATE_LINK_LIBRARIES TruthUtils
                     LINK_LIBRARIES FlowAfterburnerLib )

# Install files from the package:
atlas_install_joboptions( share/*.py )

atlas_install_python_modules( python/*.py )
