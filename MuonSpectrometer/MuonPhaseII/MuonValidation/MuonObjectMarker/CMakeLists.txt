# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: MuonObjectMarker
################################################################################

# Declare the package name:
atlas_subdir( MuonObjectMarker )



atlas_add_component( MuonObjectMarker
                     src/components/*.cxx src/*.cxx 
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTruthHelpers
                                    xAODMuonSimHit xAODMuonPrepData MuonPatternEvent MuonPatternHelpers MuonSegment
                                    DerivationFrameworkMuonsLib MuonReadoutGeometryR4 xAODMuon xAODMuonSimHit
                                    TrkCompetingRIOsOnTrack xAODMuonViews)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
