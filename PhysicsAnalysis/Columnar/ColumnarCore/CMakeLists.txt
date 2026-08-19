# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarCore )

set (extra_libs xAODBase AsgDataHandlesLib)

set (extra_libs ${extra_libs} AsgTools)

set (extra_libs ${extra_libs} ColumnarInterfacesLib)

# Add the shared library:
atlas_add_library (ColumnarCoreLib
  ColumnarCore/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarCore
  LINK_LIBRARIES ${extra_libs} SGCoreLib ColumnarModeDefaultLib
  PRIVATE_LINK_LIBRARIES CxxUtils)

if (XAOD_STANDALONE)
  # Add the dictionary (for AnalysisBase only):
  atlas_add_dictionary (ColumnarCoreDict
    ColumnarCore/ColumnarCoreDict.h
    ColumnarCore/selection.xml
    LINK_LIBRARIES ColumnarCoreLib)
endif ()

if ("${COLUMNAR_DEFAULT_ACCESS_MODE}" EQUAL 2)

  atlas_add_test( gt_AccessorsArray
    SOURCES test/gt_AccessorsArray.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarCoreLib ColumnarEventInfoLib AsgTestingLib
    POST_EXEC_SCRIPT noerror.sh )

endif()
