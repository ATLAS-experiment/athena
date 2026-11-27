# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BCMPrimeGeoModelXml )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelTools )
# Component(s) in the package:
atlas_add_component( BCMPrimeGeoModelXml
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelTools::GeoModelXml GeoModelTools::ExpressionEvaluator
                                            GaudiKernel DetDescrConditions InDetGeoModelUtils InDetSimEvent BCMPrimeReadoutGeometry )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
