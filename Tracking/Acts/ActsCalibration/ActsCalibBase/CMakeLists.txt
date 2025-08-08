# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsCalibBase )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson )

atlas_add_library( ActsCalibBase
                   ActCalibBase/*.h
                   INTERFACE
                   PUBLIC_HEADERS ActsCalibBase
                   LINK_LIBRARIES Acts::Core GaudiKernel GeoPrimitives)              