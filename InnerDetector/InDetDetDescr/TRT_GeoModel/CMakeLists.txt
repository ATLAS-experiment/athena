# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TRT_GeoModel )

# External dependencies:
find_package( GeoModelIO )
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( TRT_GeoModel
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead
                                    AthenaKernel GeoModelInterfaces GeoModelUtilities GeoPrimitives GaudiKernel 
                                    TRT_ConditionsServicesLib InDetGeoModelUtils ReadoutGeometryBase 
                                    InDetReadoutGeometry TRT_ReadoutGeometry CxxUtils SGTools StoreGateLib 
                                    AthenaPoolUtilities DetDescrConditions IdDictDetDescr TRT_ConditionsData 
                                    InDetIdentifier RDBAccessSvcLib PathResolver)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( TRT_GMConfig_test
                SCRIPT test/TRT_GMConfig_test.py
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 600 )
