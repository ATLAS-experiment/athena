# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( StorageSvc )

# External dependencies:
find_package( ROOT COMPONENTS Core Thread )
find_package( UUID )

# Component(s) in the package:
atlas_add_library( StorageSvc src/*.cpp
   PUBLIC_HEADERS StorageSvc
   INCLUDE_DIRS ${UUID_INCLUDE_DIRS}
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${UUID_LIBRARIES} AthenaBaseComps CxxUtils DataModelRoot
   GaudiKernel PersistentDataModel
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel )

atlas_add_dictionary( StorageSvcDict
   StorageSvc/dict.h StorageSvc/dict.xml
   LINK_LIBRARIES StorageSvc )


# The test(s) of the package:
atlas_add_test( StorageSvc_mtShape_test
   SOURCES test/StorageSvc_mtShape_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} StorageSvc
   POST_EXEC_SCRIPT noerror.sh )

if( NOT SIMULATIONBASE AND NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
    atlas_add_test( StorageSvc_getGuid_test
       SOURCES test/StorageSvc_getGuid_test.cxx
       LINK_LIBRARIES StorageSvc )
endif()

set( _storageTestDictDir ${CMAKE_CURRENT_SOURCE_DIR}/test/TestDictionary )

# Component(s) in the package:
atlas_add_dictionary( test_StorageTestDictionary
   test/TestDictionary/dict_all.h test/TestDictionary/classes.xml
   INCLUDE_DIRS ${_storageTestDictDir}
   LINK_LIBRARIES AthContainers
   NO_ROOTMAP_MERGE )

# Helper function setting up the tests of the package:
function( _add_test name )
   # Parse the optional argument(s):
   cmake_parse_arguments( ARG "" "" "DEPENDS" ${ARGN} )
   # Set up the test:
   atlas_add_test( "StorageSvc_${name}_test"
      SOURCES test/${name}.cpp
      INCLUDE_DIRS ${_storageTestDictDir}
      LINK_LIBRARIES PoolSvcLib TestTools StorageSvc AthContainers PersistentDataModel AthenaKernel GaudiKernel
      # Need to explicitly link against ROOT libraries to suppress
      # spurious ubsan warnings.
      ${ROOT_LIBRARIES}
      ENVIRONMENT "ROOT_INCLUDE_PATH=$ENV{ROOT_INCLUDE_PATH}:${_storageTestDictDir}"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN "FID|0x[0-9a-f]{4,}|RNTuple.*(DEBUG|VERBOSE)"
      DEPENDS test_StorageTestDictionary ${ARG_DEPENDS} )
endfunction( _add_test )

_add_test( BasicFunctionality )
_add_test( AuxStore  )
_add_test( MultipleIterators )
_add_test( ParallelReadWrite )
_add_test( TransientMembers )

