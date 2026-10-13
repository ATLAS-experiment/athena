# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarCore )

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

set (extra_libs xAODBase AsgDataHandlesLib)

set (extra_libs ${extra_libs} AsgTools)

set (extra_libs ${extra_libs} ColumnarInterfacesLib)

# IMPORTANT: ColumnarModeLib **needs** to be linked privately to
# ColumnarCoreLib, and every user needs to publicly link to the version
# of ColumnarModeLib they are actually using. Do **not** change this to
# a public link dependency here.
atlas_add_library (ColumnarCoreLib
  ColumnarCore/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarCore
  LINK_LIBRARIES ${extra_libs} SGCoreLib
  PRIVATE_LINK_LIBRARIES CxxUtils ColumnarModeLib)

if (XAOD_STANDALONE)
  # Add the dictionary (for AnalysisBase only):
  atlas_add_dictionary (ColumnarCoreDict
    ColumnarCore/ColumnarCoreDict.h
    ColumnarCore/selection.xml
    LINK_LIBRARIES ColumnarCoreLib ColumnarModeLib)
endif ()

# This test is specific to ColumnarModeArray. Instead of hard-coding a
# specific suffix, I check the modes defined together with their suffix,
# and use the corresponding suffix for the library name.
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
  atlas_add_test( gt_AccessorsArray
    SOURCES test/gt_AccessorsArray.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarCoreLib ColumnarModeLib${CMODE_SUFFIX} ColumnarEventInfoLib AsgTestingLib
    POST_EXEC_SCRIPT noerror.sh )
endif()
