# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonActsDump )


# Component(s) in the package:
atlas_add_component( MuonActsDump
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     xAODMuonSimHit StoreGateLib MuonTruthHelpers MuonPRDTestLib
                                     MuonPatternEvent MuonSpacePoint xAODMuon MuonTesterTreeLib)
                                     
atlas_install_python_modules( python/*.py)
