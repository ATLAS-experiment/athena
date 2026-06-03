# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthExOnnxRuntime )

find_package( onnxruntime )
find_package( fmt )

# Component(s) in the package.
atlas_add_component( AthExOnnxRuntime
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AthenaBaseComps GaudiKernel PathResolver
   AthOnnxInterfaces AthOnnxUtilsLib AsgServicesLib ${FMT_LIBRARIES}
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
   SCRIPT athena --evtMax 2 test_AthExOnnxRuntimeExampleCfg.pkl
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   DEPENDS AthExOnnxRuntimeTest
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthInferORTTest
   SCRIPT python -m AthExOnnxRuntime.AthExOnnxRuntime_test_infer
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthInferORTTest_pkl
   SCRIPT athena --evtMax 5 test_AthInferORTExampleCfg.pkl
   LOG_IGNORE_PATTERN "ERROR Locating dev file"
   DEPENDS AthInferORTTest
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthAsyncInferORTTest
   SCRIPT python -m AthExOnnxRuntime.AthExOnnxRuntime_test_async_infer
   LOG_IGNORE_PATTERN "ERROR Locating dev file"  # OK for this example unit test
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AthAsyncInferORTTest_pkl
   SCRIPT athena --evtMax 5 --threads=4 --CA test_AsyncInferORTExampleCfg.pkl
   LOG_IGNORE_PATTERN "ERROR Locating dev file"  # OK for this example unit test
   DEPENDS AthAsyncInferORTTest
   POST_EXEC_SCRIPT noerror.sh )
