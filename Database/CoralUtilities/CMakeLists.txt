# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CoralUtilities )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( chai )
find_package( nlohmann_json )
find_package( ROOT COMPONENTS Core RIO Tree )
find_package( ZLIB )
find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_library( CoralUtilitiesLib
   CoralUtilities/*.h src/*.cxx
   PUBLIC_HEADERS CoralUtilities
   LINK_LIBRARIES nlohmann_json::nlohmann_json chai::container AthenaPoolUtilities
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                        ${ZLIB_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} ${ZLIB_LIBRARIES}
                          CxxUtils )

atlas_add_test( ChaiCoralConverter_test
   SOURCES test/ChaiCoralConverter_test.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
   LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaPoolUtilities CoralUtilitiesLib CxxUtils Boost::unit_test_framework
   POST_EXEC_SCRIPT noerror.sh )
