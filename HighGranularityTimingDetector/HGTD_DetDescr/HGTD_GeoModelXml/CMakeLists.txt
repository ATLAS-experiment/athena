# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_GeoModelXml )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelTools )

# Component(s) in the package:
atlas_add_component( HGTD_GeoModelXml
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelTools::GeoModelXml
                                            GeoModelTools::ExpressionEvaluator GaudiKernel HGTD_ReadoutGeometry 
                                            InDetGeoModelUtils InDetSimEvent )

atlas_install_python_modules( python/*.py )
