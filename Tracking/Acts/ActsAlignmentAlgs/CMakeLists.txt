# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsAlignmentAlgs )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelHelpers )

# Component(s) in the package:
atlas_add_component( ActsAlignmentAlgs
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS  ${GEOMODEL_INCLUDE_DIRS} 
                     LINK_LIBRARIES  ${GEOMODEL_LIBRARIES}  ActsGeometryInterfacesLib GeoPrimitives
                                    AthenaBaseComps AthenaKernel CxxUtils EventInfo GaudiKernel StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

