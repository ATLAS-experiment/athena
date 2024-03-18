# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondSvc )

# Component(s) in the package:
atlas_add_library( MuonCondSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondSvc
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaPoolUtilities StoreGateLib )

atlas_add_component( MuonCondSvc
                     src/components/*.cxx
                     LINK_LIBRARIES MuonCondSvcLib )

