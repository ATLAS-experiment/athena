# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarHelperTools )

# This package currently only provides tools that make sense in the
# columnar array mode (ColumnarModeArray). In xAOD mode element links
# are represented directly, so there is nothing to build here.
if (NOT "${COLUMNAR_DEFAULT_ACCESS_MODE}" EQUAL 2)
  return()
endif()

find_package( GTest )

# Add the shared library:
atlas_add_library( ColumnarHelperToolsLib
  ColumnarHelperTools/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarHelperTools
  LINK_LIBRARIES ColumnarCoreLib AsgTools )

atlas_add_component( ColumnarHelperTools
  src/components/*.cxx
  LINK_LIBRARIES ColumnarHelperToolsLib )

atlas_add_test( gt_ColumnarToolTests
  SOURCES test/gt_ColumnarToolTests.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib ColumnarHelperToolsLib
  POST_EXEC_SCRIPT noerror.sh )
