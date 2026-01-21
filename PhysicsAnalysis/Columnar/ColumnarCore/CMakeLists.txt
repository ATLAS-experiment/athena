# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarCore )

# Allow switching the columnar access mode via a cmake variable.  This
# is numeric as we may want to try multiple modes and this makes it
# easy to add more modes and pass them through to the preprocessor for
# conditional compilation.
SET (COLUMNAR_DEFAULT_ACCESS_MODE 0 CACHE STRING "access mode for columnar components (0: xaod, 2: array-columnar, 100: xaod-array-columnar)")
set (valid_modes 0 2 100)
if ( NOT "${COLUMNAR_DEFAULT_ACCESS_MODE}" IN_LIST valid_modes)
  message (SEND_ERROR "invalid COLUMNAR_DEFAULT_ACCESS_MODE: ${COLUMNAR_DEFAULT_ACCESS_MODE}")
endif ()
message ("using columnar mode ${COLUMNAR_DEFAULT_ACCESS_MODE}")

set (extra_libs xAODBase AsgDataHandlesLib)

set (extra_libs ${extra_libs} AsgTools)

set (extra_libs ${extra_libs} ColumnarInterfacesLib)

# Add the shared library:
atlas_add_library (ColumnarCoreLib
  ColumnarCore/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarCore
  LINK_LIBRARIES ${extra_libs})

target_compile_definitions (ColumnarCoreLib PUBLIC -DCOLUMNAR_DEFAULT_ACCESS_MODE=${COLUMNAR_DEFAULT_ACCESS_MODE})

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
    POST_EXEC_SCRIPT nopost.sh )

endif()
