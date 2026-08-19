# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarModeDefault )

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

# Add the shared library:
atlas_add_library (ColumnarModeDefaultLib
  ColumnarModeDefault/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarModeDefault)

target_compile_definitions (ColumnarModeDefaultLib INTERFACE -DCOLUMNAR_DEFAULT_ACCESS_MODE=${COLUMNAR_DEFAULT_ACCESS_MODE})
