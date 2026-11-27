# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( StripGeoModelXml )

find_package( GeoModelTools )
find_package( GeoModelCore )
# Component(s) in the package:
atlas_add_component( StripGeoModelXml
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelTools::ExpressionEvaluator GeoModelTools::GeoModelXml CxxUtils
                     GaudiKernel DetDescrConditions InDetGeoModelUtils InDetSimEvent SCT_ReadoutGeometry )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( ITkStripGMConfig_test
                SCRIPT test/ITkStripGMConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )
