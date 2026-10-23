# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarToolWrapper )
find_package( GTest )

# FIX ME: These settings should come from the Project configuration
if (COLUMNAR_ANALYSIS)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeArray;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
elseif (XAOD_STANDALONE)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
else()
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD")
  SET (COLUMNAR_MODE_SUFFIX_LIST "")
endif()

# Add the shared library. This only depends on the columnar interfaces,
# not on the columnar mode, so there is only a single version of it for
# all columnar modes. Do **not** add a dependency on ColumnarModeLib or
# ColumnarCoreLib here.
atlas_add_library (ColumnarToolWrapperLib
ColumnarToolWrapper/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarToolWrapper
  LINK_LIBRARIES ColumnarInterfacesLib
  PRIVATE_LINK_LIBRARIES CxxUtils )

# These tests are specific to ColumnarModeArray. Instead of hard-coding
# a specific suffix, I check the modes defined together with their
# suffix, and use the corresponding suffix for the library name.
list(FIND COLUMNAR_MODE_NAME_LIST ColumnarModeArray idx)
if(idx GREATER -1)
  # COLUMNAR_MODE_SUFFIX_LIST may be a single-element list consisting of
  # an empty string. This protects against that.
  list(LENGTH COLUMNAR_MODE_SUFFIX_LIST suffix_len)
  if(idx LESS suffix_len)
    list(GET COLUMNAR_MODE_SUFFIX_LIST ${idx} CMODE_SUFFIX)
  else()
    set(CMODE_SUFFIX "")
  endif()

  atlas_add_test( gt_ColumnarToolWrapper
    SOURCES test/gt_ColumnarToolWrapper.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperLib ColumnarCoreLib ColumnarModeLib${CMODE_SUFFIX} AsgTestingLib
    POST_EXEC_SCRIPT noerror.sh )

  # This loads the ColumnarModeArray version of the example tools from
  # the component library at run time.
  atlas_add_test( gt_RunColumnarTool
    SOURCES test/gt_RunColumnarTool.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperLib ColumnarModeLib${CMODE_SUFFIX} AsgTestingLib AsgTools
    POST_EXEC_SCRIPT noerror.sh )
endif()
