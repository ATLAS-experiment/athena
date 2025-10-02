# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsMuonDetector
################################################################################

# Declare the package name:
atlas_subdir( ActsMuonDetector )
find_package(Acts CONFIG COMPONENTS PluginGeoModel)
find_package( GeoModel COMPONENTS GeoModelValidation )

atlas_add_component( ActsMuonDetector
                     src/components/*.cxx src/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel GeoModelUtilities ActsGeometryLib MuonIdHelpersLib ActsGeoUtils
                                    GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib AthenaPoolUtilities Acts::PluginGeoModel ${GEOMODEL_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
