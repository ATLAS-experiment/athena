# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ControlTest )

# Helper variable(s):
set( _jobOPath
   "${CMAKE_CURRENT_SOURCE_DIR}/share:${CMAKE_JOBOPT_OUTPUT_DIRECTORY}" )
set( _jobOPath "${_jobOPath}:$ENV{JOBOPTSEARCHPATH}" )

# Test(s) in the package:
atlas_add_test( DataProxy_test
   SOURCES test/DataProxy_test.cxx
   LINK_LIBRARIES TestTools ToyConversionLib AthenaKernel SGTools GaudiKernel
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR
      "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" )
   set( PPSoptignore "requestRelease|" )
else()
   set( PPSoptignore )
endif()

atlas_add_test( ProxyProviderSvc_test
   SOURCES test/ProxyProviderSvc_test.cxx
   LINK_LIBRARIES TestTools ToyConversionLib AthenaKernel SGTools StoreGateLib GaudiKernel CxxUtils
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}|${PPSoptignore}^ClassIDSvc +DEBUG|Histogram.* (DEBUG|INFO)|DetectorStore +DEBUG|ToyConversionSvc +DEBUG|EventDataSvc +DEBUG|EventPersis.* +(DEBUG|INFO)|HistoryStore +DEBUG|^DataProxy +VERBOSE|^ProxyProviderSvc +DEBUG|^ServiceManager +DEBUG"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( CircularDep_test
   SOURCES test/CircularDep_test.cxx
   LINK_LIBRARIES TestTools AthContainers SGTools StoreGateLib GaudiKernel
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( SGDataLink_test
   SOURCES test/SGDataLink_test.cxx
   LINK_LIBRARIES TestTools AthLinks AthenaKernel AthContainers SGTools StoreGateLib GaudiKernel
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}|ERROR.*known CLID.*7368883"  # duplicate CLID defined by test
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( ElementLink_test
   SOURCES test/ElementLink_test.cxx
   LINK_LIBRARIES TestTools AthLinks AthContainers SGTools StoreGateLib
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( ElementLinkVector_test
   SOURCES test/ElementLinkVector_test.cxx
   LINK_LIBRARIES TestTools AthLinks AthContainers SGTools StoreGateLib GaudiKernel
   LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}|ERROR.*known CLID.*7368883"  # duplicate CLID defined by test
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR
      "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" )
   set( SGGoptignore
      "|Retrieved const handle to default object|of type EventInfo|object not modifiable when retrieved|requestRelease" )
else()
   set( SGGoptignore )
endif()

atlas_add_test( test_interactive_athena
   SCRIPT athena.py ControlTest/test_interactive_athena.py )

# Install files from the package:
atlas_install_joboptions( share/StoreGateTestCommon.txt share/DataLink_test.txt share/*.py )
