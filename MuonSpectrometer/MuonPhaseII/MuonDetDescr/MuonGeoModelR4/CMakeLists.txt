# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonGeoModelR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelR4 )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead GeoModelHelpers GeoModelValidation)

set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package(Acts CONFIG COMPONENTS Core)
    set( extra_libs ActsCore )
endif()

# Component(s) in the package:
atlas_add_library( MuonGeoModelR4Lib
                   MuonGeoModelR4/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonGeoModelR4
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel)

atlas_add_component( MuonGeoModelR4
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel StoreGateLib GeoModelUtilities 
                                    GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib AthenaPoolUtilities MuonDetDescrUtils
                                    GeoModelUtilities GaudiKernel MuonReadoutGeometryR4 StoreGateLib ${extra_libs} CxxUtils)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
