# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZDC_GeoM )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( ZDC_GeoM
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaKernel GeoModelUtilities GaudiKernel
                                    GeoModelInterfaces StoreGateLib AthenaBaseComps ZdcIdentifier ZdcConditions)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
