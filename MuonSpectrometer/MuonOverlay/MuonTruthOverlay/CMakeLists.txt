# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonTruthOverlay )

# Component(s) in the package:
atlas_add_component( MuonTruthOverlay
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps MuonSimData StoreGateLib )
