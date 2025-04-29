# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Identifier )

# External dependencies:
find_package( Boost COMPONENTS container )

# Component(s) in the package:
atlas_add_library( Identifier
   src/*.cxx
   PUBLIC_HEADERS Identifier
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} GaudiKernel )

# Force IdentifierField.cxx to be compiled with optimization, even in a debug build.
# This significantly speeds up some tests (for example MuonCondDump_TestMdtCablingDump).
# However, you may want to change this if you need to debug this package.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/IdentifierField.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
endif()

# Tests in the package:
find_package( Boost COMPONENTS unit_test_framework )

foreach( _test Identifier32 Identifier HWIdentifier IdentifierHash Identifiable
               ExpandedIdentifier IdContext IdHelper IdentifierField Range
               RangeIterator MultiRange )

   atlas_add_test( ${_test}
      SOURCES  test/${_test}_test.cxx
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
      LINK_LIBRARIES ${Boost_LIBRARIES} Identifier
      POST_EXEC_SCRIPT nopost.sh )

endforeach()
