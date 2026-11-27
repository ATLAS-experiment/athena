# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonGeoModelR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelR4 )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelIO )

set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package(Acts CONFIG COMPONENTS Core)
    set( extra_libs Acts::Core )
endif()

# Component(s) in the package:
atlas_add_library( MuonGeoModelR4Lib
                   MuonGeoModelR4/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonGeoModelR4
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers 
                                  GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead 
                                  AthenaBaseComps AthenaKernel)

atlas_add_component( MuonGeoModelR4
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers 
                                    GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead 
                                    AthenaBaseComps AthenaKernel StoreGateLib GeoModelUtilities ${extra_libs}
                                    MuonGeoModelR4Lib AthenaPoolUtilities MuonDetDescrUtils GaudiKernel 
                                    MuonReadoutGeometryR4 CxxUtils)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
