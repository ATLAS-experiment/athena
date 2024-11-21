# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( IdDict )

# Component(s) in the package:
atlas_add_library( IdDict
                   src/IdDictMgr.cxx
                   src/IdDictFieldImplementation.cxx
                   PUBLIC_HEADERS IdDict
                   LINK_LIBRARIES Identifier )

# Code in this file makes heavy use of eigen and runs orders of magnitude
# more slowly without optimization.  So force this to be optimized even
# in debug builds.  If you need to debug it you might want to change this.
# Specifying optimization via an attribute on the particular
# function didn't work, because that still didn't allow inlining.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/IdDictMgr.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()

atlas_add_test(IdDictLabel_test
  SOURCES test/IdDictLabel_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(IdDictField_test
  SOURCES test/IdDictField_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(IdDictRegion_test
  SOURCES test/IdDictRegion_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdDictSubRegion_test
  SOURCES test/IdDictSubRegion_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(IdDictRange_test
  SOURCES test/IdDictRange_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdDictRangeRef_test
  SOURCES test/IdDictRangeRef_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdDictReference_test
  SOURCES test/IdDictReference_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)
atlas_add_test(IdDictDictionaryRef_test
  SOURCES test/IdDictDictionaryRef_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)


atlas_add_test(IdDictDictionary_test
  SOURCES test/IdDictDictionary_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict
  POST_EXEC_SCRIPT nopost.sh 
)

atlas_add_test(IdDictMgr_test
  SOURCES test/IdDictMgr_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDict IdDictParser
  POST_EXEC_SCRIPT nopost.sh 
)