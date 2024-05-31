# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( POOLCore )

# Component(s) in the package:
atlas_add_library( POOLCore
                   src/*.cpp
                   PUBLIC_HEADERS POOLCore
                   LINK_LIBRARIES GaudiKernel )

# Install files from the package:
atlas_install_scripts( scripts/*.py )
