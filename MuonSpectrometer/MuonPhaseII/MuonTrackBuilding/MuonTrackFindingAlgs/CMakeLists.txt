# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonTrackFindingAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackFindingAlgs )

find_package( Acts COMPONENTS Core )

find_package( ROOT COMPONENTS Gpad Graf Core Hist )

atlas_add_component( MuonTrackFindingAlgs
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4 ActsCore
                                     xAODTracking CxxUtils MuonSpacePoint ActsGeometryInterfacesLib MuonDetDescrUtils
                                     MuonPatternEvent ActsGeometryLib MagFieldConditions MuonRecToolInterfacesR4
                                     MuonPatternHelpers MuonTrackEvent
                                     MuonVisualizationHelpersR4 MuonTruthHelpers ${ROOT_LIBRARIES})


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
