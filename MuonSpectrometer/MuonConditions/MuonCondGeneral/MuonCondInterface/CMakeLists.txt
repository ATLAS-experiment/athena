# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondInterface )

# Component(s) in the package:
atlas_add_library( MuonCondInterface
                   PUBLIC_HEADERS MuonCondInterface
                   LINK_LIBRARIES GaudiKernel )
