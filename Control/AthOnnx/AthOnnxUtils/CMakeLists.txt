# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthOnnxUtils )

# External dependencies.
find_package( onnxruntime )

# AthenaBaseComps if in Athena
if (XAOD_STANDALONE)
  set(COND_ATHENA_BASE_COMPS "")
else()
  set(COND_ATHENA_BASE_COMPS AthenaBaseComps)
endif()

# Component(s) in the package.
atlas_add_library( AthOnnxUtilsLib
   AthOnnxUtils/*.h 
   src/*.cxx
   PUBLIC_HEADERS AthOnnxUtils
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} ${COND_ATHENA_BASE_COMPS}
)
