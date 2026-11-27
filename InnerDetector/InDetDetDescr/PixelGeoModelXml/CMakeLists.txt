# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelGeoModelXml )


find_package( GeoModelCore )
find_package( GeoModelTools )
# Component(s) in the package:
atlas_add_library( PixelGeoModelXmlLib
                   src/*.cxx
                   PUBLIC_HEADERS PixelGeoModelXml
                   LINK_LIBRARIES InDetGeoModelUtils PixelReadoutGeometryLib
                   PRIVATE_LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelTools::GeoModelXml 
                                          GeoModelTools::ExpressionEvaluator GaudiKernel DetDescrConditions 
                                          InDetSimEvent )

atlas_add_component( PixelGeoModelXml
                     src/components/*.cxx
                     LINK_LIBRARIES PixelGeoModelXmlLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package:
atlas_add_test( ITkPixelGMConfig_test
                SCRIPT test/ITkPixelGMConfig_test.py
                POST_EXEC_SCRIPT noerror.sh )
