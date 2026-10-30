# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonTrackFindingTools
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackFindingTools )

find_package( Acts COMPONENTS Core )


atlas_add_library( MuonTrackFindingToolsLib
                   Root/Chi2Fitter.cxx 
                   LINK_LIBRARIES Acts::Core
                   PUBLIC_HEADERS MuonTrackFindingTools )
atlas_add_component( MuonTrackFindingTools
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4
                                    Acts::Core xAODTracking CxxUtils MuonSpacePoint ActsGeometryInterfacesLib 
                                    MuonDetDescrUtils ActsToolInterfacesLib MuonTrackFindingToolsLib
                                    MuonPatternEvent ActsGeometryLib MuonRecToolInterfacesR4 
                                    ActsCalibratorsLib MuonTruthHelpers FourMomUtils)

