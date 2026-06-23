# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ALFA_Geometry )

# External dependencies:
find_package( CLHEP )
find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( ALFA_Geometry
                   src/*.cxx
                   PUBLIC_HEADERS ALFA_Geometry
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaKernel GaudiKernel GeoModelInterfaces RDBAccessSvcLib
                   PRIVATE_LINK_LIBRARIES PathResolver StoreGateLib )
                   
atlas_add_test( ALFA_ConfigParams_test
  SOURCES test/ALFA_ConfigParams_test.cxx
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ALFA_Geometry ${Boost_LIBRARIES}
  POST_EXEC_SCRIPT nopost.sh
)

# Install files from the package:
atlas_install_data( share/metrology/*.dat )
