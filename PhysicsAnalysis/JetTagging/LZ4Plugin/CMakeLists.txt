# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( LZ4Plugin )

# find lz4
find_package( lz4 1.9.3 )

# Standard ATLAS stuff: make a library
find_package( HDF5 1.10.1 COMPONENTS CXX HL )
atlas_add_library( LZ4PluginLib
  src/H5Zlz4.c
  src/addLz4ToPlist.cxx
  PUBLIC_HEADERS LZ4Plugin
  INCLUDE_DIRS ${HDF5_INCLUDE_DIRS} ${lz4_INCLUDE_DIRS}
  LINK_LIBRARIES ${HDF5_LIBRARIES} ${lz4_LIBRARIES} )

atlas_add_test( LZ4PluginTest
  SOURCES test/LZ4PluginTest.cxx
  INCLUDE_DIRS ${HDF5_INCLUDE_DIRS}
  LINK_LIBRARIES LZ4PluginLib ${HDF5_LIBRARIES} CxxUtils
  POST_EXEC_SCRIPT nopost.sh )
