# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PhotonVertexSelection )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO Physics TMVA )
find_package( onnxruntime )

# Add the shared library:
# Declare package as a library
# Note the convention that library names get "Lib" suffix
# Any package you depend on you should add
# to LINK_LIBRARIES line below (see the example)
# Libraries in the package:
atlas_add_library( PhotonVertexSelectionLib
   PhotonVertexSelection/*.h Root/*.cxx
   PUBLIC_HEADERS PhotonVertexSelection
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AsgDataHandlesLib AsgTools xAODEgamma xAODEventInfo xAODMetaData AnaAlgorithmLib PATCoreLib
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
      POST_EXEC_SCRIPT noerror.sh )
endif()
