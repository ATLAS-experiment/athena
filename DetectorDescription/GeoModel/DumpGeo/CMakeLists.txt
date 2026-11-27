# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>

# Declare the package name.
atlas_subdir( DumpGeo )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelIO )


# Component(s) in the package.
atlas_add_component( DumpGeo
   DumpGeo/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelWrite 
                  AthenaBaseComps AthenaKernel CxxUtils GaudiKernel GeoModelUtilities )

# Install files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests.
atlas_add_test( DumpGeoConfig
   SCRIPT python -m unittest -v DumpGeo.DumpGeoConfig_test
   POST_EXEC_SCRIPT noerror.sh )
