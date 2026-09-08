# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonTrackFindingTools
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackFindingTools )

find_package( Acts COMPONENTS Core )

atlas_add_component( MuonTrackFindingTools
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4
                                    Acts::Core xAODTracking CxxUtils MuonSpacePoint ActsGeometryInterfacesLib MuonDetDescrUtils
                                    MuonPatternEvent ActsGeometryLib MuonRecToolInterfacesR4 
                                    ActsCalibratorsLib MuonTruthHelpers FourMomUtils)

