# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PhotonVertexSelection )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO Physics TMVA )

# Libraries in the package:
atlas_add_library( PhotonVertexSelectionLib
   PhotonVertexSelection/*.h Root/*.cxx
   PUBLIC_HEADERS PhotonVertexSelection
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools xAODEgamma xAODEventInfo xAODMetaData AnaAlgorithmLib PATCoreLib
   xAODTracking
   PRIVATE_LINK_LIBRARIES PathResolver egammaUtils CxxUtils)

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
