# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TruthD3PDMaker )

# External dependencies:
find_package( HepPDT )

# Component(s) in the package:
atlas_add_component( TruthD3PDMaker
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS}
                     LINK_LIBRARIES TruthUtils ${HEPPDT_LIBRARIES} AtlasHepMCLib AthenaBaseComps AthenaKernel Navigation xAODTruth GaudiKernel GeneratorObjects TruthUtils D3PDMakerInterfaces D3PDMakerUtils MCTruthClassifierLib JetEvent TrkToolInterfaces ParticleTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

