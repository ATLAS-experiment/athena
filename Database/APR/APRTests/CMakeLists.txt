# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( APRTests )

# External dependencies:
find_package( CppUnit )
find_package( ROOT )

# Extend the include directories for all components:
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/TestDictionary )

# Component(s) in the package:
atlas_add_dictionary( test_TestDictionaryDict
   TestDictionary/dict_all.h TestDictionary/classes.xml
   NO_ROOTMAP_MERGE )

# Helper function setting up the tests of the package:
function( _add_test name )
   # Parse the optional argument(s):
   cmake_parse_arguments( ARG "" "DICTHEADER" "DEPENDS" ${ARGN} )
   # Create a dictionary if one was requested:
   if( ARG_DICTHEADER )
      atlas_add_dictionary( test_${name}Dict
         ${ARG_DICTHEADER} ${name}/classes.xml
         LINK_LIBRARIES PersistencySvc
         StorageSvc AthContainers
         NO_ROOTMAP_MERGE )
   endif()
   # Set up the test:
   atlas_add_test( "${name}_test"
      SOURCES ${name}/*.cpp
      INCLUDE_DIRS ${CPPUNIT_INCLUDE_DIRS}
      LINK_LIBRARIES ${CPPUNIT_LIBRARIES} PersistencySvc TestTools
      StorageSvc AthContainers  FileCatalog
      PersistentDataModel TestTools AthenaKernel
      # Need to explicitly link against ROOT libraries to suppress
      # spurious ubsan warnings.
      ${ROOT_LIBRARIES}
      ENVIRONMENT "ROOT_INCLUDE_PATH=$ENV{ROOT_INCLUDE_PATH}:${CMAKE_CURRENT_SOURCE_DIR}/TestDictionary"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN "POOL_CATALOG is not defined|using default|New file created|FID|0x[0-9a-f]{4,}|APRFileCatalog.*DEBUG|RNTuple.*(DEBUG|VERBOSE)"
      DEPENDS ${ARG_DEPENDS} )
endfunction( _add_test )

# Declare the test(s) of the package:
# Make them all run sequentially so PRE_EXEC cleanups do not interfere
_add_test( FileCatalog_Functionality )
_add_test( StorageSvc_BasicFunctionality )
_add_test( StorageSvc_AuxStore  DICTHEADER StorageSvc_AuxStore/AuxStoreTest.h )
_add_test( StorageSvc_MultipleIterators )
_add_test( StorageSvc_ParallelReadWrite )
_add_test( StorageSvc_TransientMembers DICTHEADER StorageSvc_TransientMembers/TestClassWithTransients.h )
_add_test( PersistencySvc_Functionality )
_add_test( PersistencySvc_ExtraFunctionality )
_add_test( PersistencySvc_FileCatalogOperations )
_add_test( PersistencySvc_FileOpenWithoutCatalog )
_add_test( PersistencySvc_Parameters )
_add_test( PersistencySvc_NoClassID DICTHEADER PersistencySvc_NoClassID/MyTestClass.h )
_add_test( OpenFileDifferentPaths )

# Clean up:
unset( _add_test )
