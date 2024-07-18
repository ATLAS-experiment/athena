# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PhotonVertexSelection )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO Physics TMVA )
find_package( onnxruntime REQUIRED )

# set libraries and includes for onnx, then print message about the paths
set(ALL_ONNX_LIBS ${onnxruntime_LIBRARY} ${onnxruntime_LIBRARIES} ${ONNXRUNTIME_LIBRARIES})
message(NOTICE "-- HyyVertexInference/CMakeLists: Using onnx libs=${ALL_ONNX_LIBS}")
set(ALL_ONNX_INCS ${onnxruntime_INCLUDE_DIR} ${ONNXRUNTIME_INCLUDE_DIRS})
message(NOTICE "-- HyyVertexInference/CMakeLists: Using onnx incs=${ALL_ONNX_INCS}")

# Add the shared library:
# Declare package as a library
# Note the convention that library names get "Lib" suffix
# Any package you depend on you should add
# to LINK_LIBRARIES line below (see the example)
# Libraries in the package:
atlas_add_library( PhotonVertexSelectionLib
   PhotonVertexSelection/*.h Root/*.cxx
   PUBLIC_HEADERS PhotonVertexSelection
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ALL_ONNX_INCS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${ALL_ONNX_LIBS} AsgDataHandlesLib AsgTools xAODEgamma xAODEventInfo xAODMetaData AnaAlgorithmLib PATCoreLib
   xAODTracking
   PRIVATE_LINK_LIBRARIES PathResolver egammaUtils CxxUtils)

# if you add athena components (tools, algorithms) to this package
# these lines are needed so you can configure them in joboptions
if( NOT XAOD_STANDALONE )
   atlas_add_component( PhotonVertexSelection
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES xAODEgamma xAODTracking AthContainers AthenaBaseComps
      GaudiKernel PhotonVertexSelectionLib )
endif()

atlas_add_dictionary( PhotonVertexSelectionDict
   PhotonVertexSelection/PhotonVertexSelectionDict.h
   PhotonVertexSelection/selection.xml
   LINK_LIBRARIES PhotonVertexSelectionLib )

# Install python modules
atlas_install_python_modules( python/*.py )

# Test(s) in the package:
if( NOT XAOD_ANALYSIS )
   atlas_add_test( PhotonVertexSelection_test
      SCRIPT test/testPhotonVertexSelection.py
      PROPERTIES TIMEOUT 300
      POST_EXEC_SCRIPT nopost.sh )
endif()