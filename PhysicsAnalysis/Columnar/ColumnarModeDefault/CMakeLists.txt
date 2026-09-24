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

# Add the shared library for xAOD mode. By convention this doesn't have
# a suffix, so that existing code does not need to change.
atlas_add_library (ColumnarModeDefaultLib
  ColumnarModeDefault/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarModeDefault)

target_compile_definitions (ColumnarModeDefaultLib INTERFACE -DCOLUMNAR_DEFAULT_ACCESS_MODE=${COLUMNAR_DEFAULT_ACCESS_MODE} -DCOLUMNAR_MODE_SUFFIX=)

# Add the shared library for array mode. This has an `_array` suffix,
# which is essentially arbitrary, but matches the name in the columnar
# documentation. Libraries linking this should by convention use the
# same postfix throughout.
atlas_add_library (ColumnarModeDefaultLib_array
  ColumnarModeDefault/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarModeDefault)

target_compile_definitions (ColumnarModeDefaultLib_array INTERFACE -DCOLUMNAR_DEFAULT_ACCESS_MODE=2 -DCOLUMNAR_MODE_SUFFIX=_array)
