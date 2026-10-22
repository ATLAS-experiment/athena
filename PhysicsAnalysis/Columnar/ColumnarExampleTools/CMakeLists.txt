# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarExampleTools )

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

foreach (CMODE_NAME CMODE_SUFFIX IN ZIP_LISTS COLUMNAR_MODE_NAME_LIST COLUMNAR_MODE_SUFFIX_LIST)
  
  message ("Building ColumnarExampleTools with columnar mode ${CMODE_NAME} and suffix ${CMODE_SUFFIX}")

  # Add the shared library:
  atlas_add_library (ColumnarExampleToolsLib${CMODE_SUFFIX}
    ColumnarExampleTools/*.h Root/*.cxx
    PUBLIC_HEADERS ColumnarExampleTools
    LINK_LIBRARIES ColumnarCoreLib AsgTools ColumnarMetLib ColumnarMuonLib ColumnarTrackingLib ColumnarModeLib${CMODE_SUFFIX})

  atlas_add_component( ColumnarExampleTools${CMODE_SUFFIX}
    src/components/*.cxx
    LINK_LIBRARIES ColumnarExampleToolsLib${CMODE_SUFFIX} )

  atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
    SOURCES test/gt_ColumnarToolTests.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib ColumnarExampleToolsLib${CMODE_SUFFIX} ColumnarJetLib xAODEgamma
    POST_EXEC_SCRIPT noerror.sh )

endforeach()
