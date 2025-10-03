# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthExOnnxRuntime )

find_package( onnxruntime )

# Component(s) in the package.
atlas_add_component( AthExOnnxRuntime
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AthenaBaseComps GaudiKernel PathResolver 
   AthOnnxInterfaces AthOnnxUtilsLib AsgServicesLib
)

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test the packages
atlas_add_test( AthExOnnxRuntimeTest
   SCRIPT python -m AthExOnnxRuntime.AthExOnnxRuntime_test
   LOG_IGNORE_PATTERN "ERROR Locating dev file"  # OK for this example unit test
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthExOnnxRuntimeTest_pkl
   SCRIPT athena --evtMax 2 --CA test_AthExOnnxRuntimeExampleCfg.pkl
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   DEPENDS AthExOnnxRuntimeTest
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthInferORTTest
   SCRIPT python -m AthExOnnxRuntime.AthExOnnxRuntime_test_infer
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthInferORTTest_pkl
   SCRIPT athena --evtMax 2 --CA test_AthInferORTExampleCfg.pkl
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   DEPENDS AthInferORTTest
   POST_EXEC_SCRIPT noerror.sh )
