# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonD3PDMaker )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( MuonD3PDMaker
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthContainers AthenaBaseComps AthenaKernel AtlasHepMCLib D3PDMakerUtils FourMomUtils GaudiKernel MCTruthClassifierLib MuonIdHelpersLib MuonSimEvent TrackRecordLib TrkTrackSummary TrkTruthData TruthUtils xAODMuon xAODTracking xAODTruth )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

