# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTestFixtures )

find_package( GTest )

set (extra_libs)
if (XAOD_STANDALONE)
  set (extra_libs ComponentFactoryPreloaderLib)
endif ()

# Add the shared library:
atlas_add_library (ColumnarTestFixturesLib
ColumnarTestFixtures/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarTestFixtures
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperLib PATInterfaces AsgTestingLib
  PRIVATE_LINK_LIBRARIES ${extra_libs} ColumnarCoreLib)
