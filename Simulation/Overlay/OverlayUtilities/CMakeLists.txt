# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( OverlayUtilities )

# Component(s) in the package:
atlas_add_component( OverlayUtilities
                     src/*.cxx src/*.h
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps EventBookkeeperToolsLib xAODTracking)

# Install python modules
atlas_install_python_modules( python/*.py )
