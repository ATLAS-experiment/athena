# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCombinedAlgsR4 )

# Component(s) in the package:
atlas_add_component( MuonCombinedAlgsR4
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps StoreGateLib GaudiKernel xAODMuon 
                                    MuonTrackEvent xAODMuonViews ActsEventLib xAODTracking
                                    MuonAnalysisInterfacesLib MuonRecToolInterfacesR4
                                    ActsToolInterfacesLib InDetTrackSelectionToolLib
                                    FourMomUtils ActsCalibratorsLib
                                    MuonCombinedEvent
                                    )
atlas_install_scripts(test/runR4_MuonCombRecoSPOTTest.sh)
