# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarToolWrapper )
find_package( GTest )

# Add the shared library:
atlas_add_library (ColumnarToolWrapperLib
ColumnarToolWrapper/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarToolWrapper
  LINK_LIBRARIES ColumnarInterfacesLib
  PRIVATE_LINK_LIBRARIES CxxUtils )

# Add the test (hardcoded to ColumnarModeArray, can run in any build mode):
atlas_add_test( gt_ColumnarToolWrapper
  SOURCES test/gt_ColumnarToolWrapper.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperLib ColumnarCoreLib AsgTestingLib
  POST_EXEC_SCRIPT nopost.sh )
