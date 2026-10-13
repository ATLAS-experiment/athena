# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarMode )

# This is a hold-over from the previous implementation and should be
# removed once the transition to the dual-library setup is done: Allow
# switching the columnar access mode via a cmake variable. This is
# numeric as we may want to try multiple modes and this makes it easy to
# add more modes and pass them through to the preprocessor for
# conditional compilation.
SET (COLUMNAR_DEFAULT_ACCESS_MODE 0 CACHE STRING "access mode for columnar components (0: xaod, 2: array-columnar, 100: xaod-array-columnar)")

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

  message ("Building ColumnarMode with columnar mode ${CMODE_NAME} and suffix ${CMODE_SUFFIX}")

  # Add the shared library for the current columnar mode.
  atlas_add_library (ColumnarModeLib${CMODE_SUFFIX}
    ColumnarMode/*.h
    INTERFACE
    PUBLIC_HEADERS ColumnarMode
    DEFINITIONS COLUMNAR_MODE_SUFFIX=${CMODE_SUFFIX} COLUMNAR_MODE_NAME=${CMODE_NAME})

endforeach()
