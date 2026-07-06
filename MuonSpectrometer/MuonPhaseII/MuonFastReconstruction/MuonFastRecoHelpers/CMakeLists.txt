# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonFastRecoHelpers )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonFastRecoHelpers
                   src/*.cxx
                   PUBLIC_HEADERS MuonFastRecoHelpers
                   LINK_LIBRARIES xAODMuonPrepData MuonSpacePoint MuonFastRecoEvent MuonPatternEvent MuonReadoutGeometryR4
                                  MuonRecToolInterfacesR4 MuonDetDescrUtils MuonTrackEvent MuonTrackFindingToolsLib
                                  MuonVisualizationHelpersR4 MuonPatternHelpers ActsCalibratorsLib ActsEventLib xAODMuonViews
                                  ActsToolInterfacesLib ActsEventLib AthenaKernel StoreGateLib FourMomUtils )