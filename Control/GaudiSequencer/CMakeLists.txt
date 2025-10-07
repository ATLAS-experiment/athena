# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GaudiSequencer )

# Component(s) in the package:
atlas_add_component( GaudiSequencer
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel )

# Install files from the package:
atlas_install_python_modules( python/*.py )

