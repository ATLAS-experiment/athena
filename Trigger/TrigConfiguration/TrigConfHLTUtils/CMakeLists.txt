# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.

# Declare the package name:
atlas_subdir( TrigConfHLTUtils )
find_package( Threads )

# Component(s) in the package:
atlas_add_library( TrigConfHLTUtilsLib
  TrigConfHLTUtils/*.h Root/*.cxx
  PUBLIC_HEADERS TrigConfHLTUtils
  LINK_LIBRARIES CxxUtils )

atlas_add_executable( trigconf_string2hash
  util/trigconf_string2hash.cxx
  LINK_LIBRARIES TrigConfHLTUtilsLib ${CMAKE_THREAD_LIBS_INIT})

atlas_install_python_modules( python/*.py )

# Tests:
atlas_add_test( string2hash
  SOURCES test/test_string2hash.cxx
  LINK_LIBRARIES TrigConfHLTUtilsLib
  POST_EXEC_SCRIPT noerror.sh )
