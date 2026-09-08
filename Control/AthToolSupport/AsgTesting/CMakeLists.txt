# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AsgTesting )

# External dependencies:
find_package( Boost COMPONENTS program_options)
find_package( GTest )
find_package( GMock )
find_package( nlohmann_json )
find_package( ROOT COMPONENTS Gpad Hist ROOTDataFrame TreePlayer )
find_package( VDT )

# Component(s) in the package:
atlas_add_library( AsgTestingLib
   AsgTesting/*.h AsgTesting/*.icc Root/*.cxx
   PUBLIC_HEADERS AsgTesting
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
   LINK_LIBRARIES AsgTools AsgMessagingLib ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})

if( NOT SIMULATIONBASE )
   atlas_add_executable( compareFlatTrees
      utils/compareFlatTrees.cxx
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS}
      LINK_LIBRARIES CxxUtils ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${VDT_LIBRARIES} nlohmann_json::nlohmann_json )
endif()

atlas_add_test( gt_UnitTest_test
   SOURCES test/gt_UnitTest_test.cxx
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
   LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTools AsgMessagingLib AsgTestingLib CxxUtils
   LOG_IGNORE_PATTERN "functionFail"
   POST_EXEC_SCRIPT noerror.sh )
