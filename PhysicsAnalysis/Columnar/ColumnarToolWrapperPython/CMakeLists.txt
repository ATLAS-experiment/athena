# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack
# @author Matthew Feickert
# @author Giordon Stark

atlas_subdir( ColumnarToolWrapperPython )

# Add the shared library:
atlas_add_library (ColumnarToolWrapperPythonLib
ColumnarToolWrapperPython/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarToolWrapperPython
  LINK_LIBRARIES ColumnarToolWrapperLib PATInterfaces)

find_package( GTest )

if (XAOD_STANDALONE)
  atlas_add_test( gt_PythonToolHandle
    SOURCES test/gt_PythonToolHandle.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperPythonLib AsgTestingLib ColumnarExampleToolsLib
    POST_EXEC_SCRIPT nopost.sh )
endif()
