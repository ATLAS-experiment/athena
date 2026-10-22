# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTestFixtures )

find_package( GTest )
find_package( nlohmann_json )
find_package( ROOT COMPONENTS Core RIO ROOTNTuple )
set (extra_libs)
if (XAOD_STANDALONE)
  set (extra_libs )
else()
  set (extra_libs POOLRootAccessLib)
endif ()

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

# FIX ME: we should probably not have a loop here, since all library
# versions are the same. However, some code is still linking the version
# with a suffix.
foreach (CMODE_NAME CMODE_SUFFIX IN ZIP_LISTS COLUMNAR_MODE_NAME_LIST COLUMNAR_MODE_SUFFIX_LIST)
  message ("Building ColumnarTestFixtures with columnar mode ${CMODE_NAME} and suffix ${CMODE_SUFFIX}")

  # Add the shared library:
  # WARNING: Do **NOT** change the ColumnarModeLib dependency to public.
  # If you get errors because you are missing the ColumnarModeLib, link
  # it in your own code. However, you are unlikely to need to link
  # ColumnarModeLib, since you ought to be pulling in ColumnarModeLib
  # from your tool code library.
  atlas_add_library (ColumnarTestFixturesLib${CMODE_SUFFIX}
  ColumnarTestFixtures/*.h Root/*.cxx
    PUBLIC_HEADERS ColumnarTestFixtures
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} ROOT::ROOTNTuple ColumnarToolWrapperLib PATInterfaces AsgTestingLib xAODJet xAODMissingET
    PRIVATE_LINK_LIBRARIES ${extra_libs} ColumnarCoreLib nlohmann_json::nlohmann_json xAODCaloEvent xAODTracking xAODMuon xAODEventInfo xAODEgamma xAODRootAccess ColumnarModeLib)

endforeach()

# FIX ME: This should switch to the dual-library mechanism once the base
# libraries are
atlas_add_test( gt_MultiToolTest${CMODE_SUFFIX}
                SOURCES test/gt_MultiToolTest.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib
                              ElectronPhotonFourMomentumCorrectionLib
                              ElectronEfficiencyCorrectionLib
                POST_EXEC_SCRIPT noerror.sh )
