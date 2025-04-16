# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonTrackFindingTools
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackFindingTools )

atlas_add_component( MuonTrackFindingTools
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4
                                     xAODTracking CxxUtils MuonSpacePoint ActsGeometryInterfacesLib MuonDetDescrUtils
                                     MuonPatternEvent ActsGeometryLib MuonRecToolInterfacesR4)


# Install files from the package:
#atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
