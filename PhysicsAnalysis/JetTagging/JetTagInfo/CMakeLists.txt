# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetTagInfo )

# Component(s) in the package:
atlas_add_library( JetTagInfo
                   src/*.cxx
                   PUBLIC_HEADERS JetTagInfo
                   PRIVATE_LINK_LIBRARIES GaudiKernel )
