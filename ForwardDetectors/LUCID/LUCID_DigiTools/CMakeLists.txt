# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LUCID_DigiTools )

# Component(s) in the package:
atlas_add_library( LUCID_DigiTools
                   src/*.cxx
                   PUBLIC_HEADERS LUCID_DigiTools
                   PRIVATE_LINK_LIBRARIES GaudiKernel )

