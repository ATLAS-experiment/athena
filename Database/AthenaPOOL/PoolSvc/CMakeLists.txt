# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PoolSvc )


atlas_add_library( PoolSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS PoolSvc
                   LINK_LIBRARIES AthenaBaseComps CxxUtils DataModelRoot GaudiKernel PathResolver PersistentDataModel StorageSvc )

# Component(s) in the package:
atlas_add_component( PoolSvc
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils DataModelRoot GaudiKernel PathResolver PersistentDataModel PoolSvcLib StorageSvc )

atlas_add_executable( pool_extractFileIdentifier
   utilities/pool_extractFileIdentifier.cxx
   LINK_LIBRARIES CxxUtils PoolSvcLib StorageSvc )

atlas_add_executable( pool_insertFileToCatalog
   utilities/pool_insertFileToCatalog.cxx
   LINK_LIBRARIES CxxUtils PoolSvcLib StorageSvc )

atlas_add_executable( FCcmd
   utilities/FCcmd.cxx
   LINK_LIBRARIES PoolSvcLib StorageSvc )

# Install files from the package:
atlas_install_scripts( scripts/*.py )

atlas_add_test( FCcmd
   SCRIPT test/FCcmd_test.sh )


set( _testDictDir ${CMAKE_CURRENT_SOURCE_DIR}/test/TestDictionary )

# dictionary with basic classes for the tests
atlas_add_dictionary( test_TestDictionaryDict
   test/TestDictionary/dict_all.h 
   test/TestDictionary/classes.xml
   INCLUDE_DIRS ${_testDictDir}
   NO_ROOTMAP_MERGE )

# Helper function setting up the tests of the package:
function( _add_test name )
   # Parse the optional argument(s):
   cmake_parse_arguments( ARG "" "DICTHEADER" "DEPENDS" ${ARGN} )
   # Set up the test:
   atlas_add_test( "${name}_test"
      SOURCES test/${name}.cpp test/PersTestBase.cpp
      INCLUDE_DIRS ${CPPUNIT_INCLUDE_DIRS} ${_testDictDir}
      LINK_LIBRARIES ${CPPUNIT_LIBRARIES} PoolSvcLib TestTools StorageSvc PersistentDataModel AthenaKernel AthenaBaseComps
      ENVIRONMENT "ROOT_INCLUDE_PATH=$ENV{ROOT_INCLUDE_PATH}:${_testDictDir}"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN "POOL_CATALOG is not defined|using default|New file created|FID|0x[0-9a-f]{4,}|APRFileCatalog.*DEBUG|RNTuple.*(DEBUG|VERBOSE)"
      DEPENDS test_TestDictionaryDict $ARG_DEPENDS )
endfunction( _add_test )

_add_test( Persistency_Functionality )
_add_test( Persistency_ExtraFunctionality )
_add_test( Persistency_NoClassID )
_add_test( Persistency_Parameters )
_add_test( FileCatalogOperations )
_add_test( FileOpenWithoutCatalog )
_add_test( FileCatalog_Functionality )
