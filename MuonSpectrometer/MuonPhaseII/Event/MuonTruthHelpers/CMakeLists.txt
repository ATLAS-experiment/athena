# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePoint
################################################################################

# Declare the package name:
atlas_subdir( MuonTruthHelpers )


atlas_add_library( MuonTruthHelpers
                   src/*.cxx
                   PUBLIC_HEADERS MuonTruthHelpers
                   LINK_LIBRARIES xAODMuonPrepData xAODMuonSimHit xAODMuon xAODTruth GeoPrimitives
                   PRIVATE_LINK_LIBRARIES MuonSpacePoint MuonPatternEvent)
