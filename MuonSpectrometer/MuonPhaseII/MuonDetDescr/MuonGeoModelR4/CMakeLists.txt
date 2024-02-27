# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonGeoModelR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelR4 )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead )

# Component(s) in the package:
atlas_add_library( MuonGeoModelR4Lib
                   src/*.cxx
                   PUBLIC_HEADERS MuonGeoModelR4
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel MuonDetDescrUtils
                                  GeoModelUtilities GaudiKernel MuonReadoutGeometryR4 StoreGateLib  CxxUtils )

atlas_add_component( MuonGeoModelR4
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel StoreGateLib GeoModelUtilities 
                                    GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib AthenaPoolUtilities)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
