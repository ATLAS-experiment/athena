# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( CaloDetDescr )

# External dependencies.
find_package( Boost )

# Component(s) in the package.
atlas_add_library( CaloDetDescrLib
   CaloDetDescr/*.h src/*.h src/*.cxx
   PUBLIC_HEADERS CaloDetDescr
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} GaudiKernel AthenaKernel AthContainers
   AthenaBaseComps CxxUtils GeoModelInterfaces Identifier CaloIdentifier
   CaloGeoHelpers GeoPrimitives LArReadoutGeometry
   PRIVATE_LINK_LIBRARIES CaloConditions StoreGateLib GeoModelUtilities)

atlas_add_component( CaloDetDescr
   src/components/*.cxx
   LINK_LIBRARIES CaloDetDescrLib )

atlas_add_dictionary( CaloDetDescrDict
   CaloDetDescr/CaloDetDescrDict.h
   CaloDetDescr/selection.xml
   LINK_LIBRARIES CaloDetDescrLib )

# Install files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test(s) in the package.
atlas_add_test( CaloConstIteratorAdaptor_test
  SOURCES test/CaloConstIteratorAdaptor_test.cxx
  LINK_LIBRARIES CaloDetDescrLib )

if( NOT GENERATIONBASE AND NOT SIMULATIONBASE)
atlas_add_test( CaloDetDescrManager_test
                SCRIPT python -m CaloDetDescr.CaloDetDescrManager_test
		PROPERTIES TIMEOUT 500
		LOG_SELECT_PATTERN "CaloDD|^----|^Sampling" )
endif()
