# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarVariant )

# Create the interface library target
atlas_add_library (ColumnarVariantLib
ColumnarVariant/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarVariant
  LINK_LIBRARIES ColumnarCoreLib )

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

find_package( GTest )

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
  atlas_add_test( gt_VariantLinkArray
    SOURCES test/gt_VariantLinkArray.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarVariantLib ColumnarCoreLib ColumnarJetLib ColumnarModeLib${CMODE_SUFFIX} AsgTestingLib
    POST_EXEC_SCRIPT noerror.sh )
endif()
