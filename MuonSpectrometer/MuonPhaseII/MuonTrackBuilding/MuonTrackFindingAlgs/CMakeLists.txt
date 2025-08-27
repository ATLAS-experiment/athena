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
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4 Acts::Core
                                     xAODTracking CxxUtils MuonSpacePoint ActsGeometryInterfacesLib MuonDetDescrUtils
                                     MuonPatternEvent MuonRecToolInterfacesR4 MuonTruthHelpers
                                     MuonTrackEvent MuonTrackFindingToolsLib ActsToolInterfacesLib ActsCalibratorsLib 
                                     MuonVisualizationHelpersR4 MuonPatternHelpers ${ROOT_LIBRARIES})


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
