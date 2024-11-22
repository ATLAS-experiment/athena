# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Identifier )

# External dependencies:
find_package( Boost COMPONENTS container unit_test_framework)

# Component(s) in the package:
atlas_add_library( Identifier
   src/*.cxx
   PUBLIC_HEADERS Identifier
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES GaudiKernel )

# Force Range.cxx to be compiled with optimization, even in a debug build.
# This significantly speeds up some tests (for example MuonCondDump_TestMdtCablingDump).
# However, you may want to change this if you need to debug this package.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/Range.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
endif()

atlas_add_test(Identifier32_test
  SOURCES  test/Identifier32_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(Identifier_test
  SOURCES  test/Identifier_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(HWIdentifier_test
  SOURCES  test/HWIdentifier_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdentifierHash_test
  SOURCES  test/IdentifierHash_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(Identifiable_test
  SOURCES  test/Identifiable_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)



atlas_add_test(ExpandedIdentifier_test
  SOURCES  test/ExpandedIdentifier_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(IdContext_test
  SOURCES  test/IdContext_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdHelper_test
  SOURCES  test/IdHelper_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)


atlas_add_test(IdentifierField_test
  SOURCES  test/IdentifierField_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(Range_test
  SOURCES test/Range_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(RangeIterator_test
  SOURCES test/RangeIterator_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(MultiRange_test
  SOURCES test/MultiRange_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier
  POST_EXEC_SCRIPT nopost.sh 
)


