# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetServMatGeoModel )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( InDetServMatGeoModel
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaBaseComps AthenaKernel CxxUtils 
                                    GaudiKernel GeoModelInterfaces GeoModelUtilities GeoPrimitives 
                                    InDetGeoModelUtils RDBAccessSvcLib SGTools StoreGateLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
