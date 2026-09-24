# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarExampleTools )

find_package( GTest )

# TO DO: the list of columnar modes should come from central
# configuration for the build project, not be hard-coded here.
foreach( CMODE "xaod" "array" )

  # TO DO: This is a bit clunky, but it avoids potentially putting empty
  # strings into a list (if there was simply a list of suffixes), which
  # can be problematic in CMake code. There may be some way to make it
  # less clunky.
  if (${CMODE} STREQUAL "xaod")
    set (CMODE_SUFFIX "")
  else ()
    set (CMODE_SUFFIX "_${CMODE}")
  endif()

  message ("Building ColumnarExampleTools with columnar mode ${CMODE} and suffix ${CMODE_SUFFIX}")

  # Add the shared library:
  atlas_add_library (ColumnarExampleToolsLib${CMODE_SUFFIX}
    ColumnarExampleTools/*.h Root/*.cxx
    PUBLIC_HEADERS ColumnarExampleTools
    LINK_LIBRARIES ColumnarCoreLib AsgTools ColumnarMetLib ColumnarMuonLib ColumnarTrackingLib ColumnarModeDefaultLib${CMODE_SUFFIX})

  atlas_add_component( ColumnarExampleTools${CMODE_SUFFIX}
    src/components/*.cxx
    LINK_LIBRARIES ColumnarExampleToolsLib${CMODE_SUFFIX} )

  atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
    SOURCES test/gt_ColumnarToolTests.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib${CMODE_SUFFIX} ColumnarExampleToolsLib${CMODE_SUFFIX} ColumnarJetLib xAODEgamma
    POST_EXEC_SCRIPT noerror.sh )

endforeach()
