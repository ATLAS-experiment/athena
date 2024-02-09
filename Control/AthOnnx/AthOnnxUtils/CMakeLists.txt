# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthOnnxUtils )

# External dependencies.
find_package( onnxruntime )

# Component(s) in the package.
atlas_add_library( AthOnnxUtilsLib
   AthOnnxUtils/*.h 
   src/*.cxx
   PUBLIC_HEADERS AthOnnxUtils
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AthenaKernel GaudiKernel AsgServicesLib 
)
