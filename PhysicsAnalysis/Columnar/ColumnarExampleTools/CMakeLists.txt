# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarExampleTools )

# Add the shared library:
atlas_add_library (ColumnarExampleToolsLib
  ColumnarExampleTools/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarExampleTools
  INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
  LINK_LIBRARIES ColumnarCoreLib AsgTools ColumnarMuonLib ColumnarTrackingLib)

atlas_add_test( gt_ColumnarToolTests
  SOURCES test/gt_ColumnarToolTests.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib ColumnarExampleToolsLib ColumnarJetLib
  POST_EXEC_SCRIPT nopost.sh )
