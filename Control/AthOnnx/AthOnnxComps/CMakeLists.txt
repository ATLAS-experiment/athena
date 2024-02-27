# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthOnnxComps )

# External dependencies.
find_package( onnxruntime )

# Libraray in the package.
atlas_add_component( AthOnnxComps
   src/*.cxx
   src/components/*.cxx
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AsgTools AsgServicesLib AthenaBaseComps AthenaKernel GaudiKernel  AthOnnxInterfaces AthOnnxUtilsLib
)

# install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
