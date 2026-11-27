# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsAlignmentAlgs )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( ActsAlignmentAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers 
                                     ActsGeometryInterfacesLib GeoPrimitives AthenaBaseComps AthenaKernel 
                                     CxxUtils EventInfo GaudiKernel StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

