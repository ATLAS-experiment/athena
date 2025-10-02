# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonSelectorTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO TMVA )
find_package( onnxruntime REQUIRED )

# Libraries in the package:
atlas_add_library( MuonSelectorToolsLib
   MuonSelectorTools/*.h Root/*.cxx
   PUBLIC_HEADERS MuonSelectorTools
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AsgTools AsgServicesLib xAODEventInfo PATCoreLib MuonAnalysisInterfacesLib AsgDataHandlesLib AthOnnxInterfaces AthOnnxUtilsLib
   PRIVATE_LINK_LIBRARIES xAODTracking PathResolver )

if( NOT XAOD_STANDALONE )
   atlas_add_component( MuonSelectorTools
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AthenaBaseComps GaudiKernel xAODCore xAODMuon MuonAnalysisInterfacesLib
      MuonSelectorToolsLib PATCoreLib )
else()
   atlas_add_component( MuonSelectorTools
      src/components/*.cxx
      LINK_LIBRARIES MuonSelectorToolsLib )
endif()

atlas_add_dictionary( MuonSelectorToolsDict
   MuonSelectorTools/MuonSelectorToolsDict.h
   MuonSelectorTools/selection.xml
   LINK_LIBRARIES MuonSelectorToolsLib )


# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( MuonSelectorToolsTester
      util/MuonSelectorToolsTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon MCTruthClassifierLib
      xAODTracking xAODCore MuonSelectorToolsLib PATCoreLib )
else()
   atlas_add_executable( MuonSelectorToolsTester
      util/MuonSelectorToolsTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} POOLRootAccessLib xAODRootAccess xAODEventInfo xAODMuon MCTruthClassifierLib
      xAODTracking xAODCore MuonSelectorToolsLib PATCoreLib )
endif()


# Test(s) in the package: 
atlas_add_test( ut_MuonSelectorToolsTester_data
   SCRIPT test/ut_MuonSelectorToolsTester_data.sh
   PROPERTIES TIMEOUT 450
   POST_EXEC_SCRIPT nopost.sh )
atlas_add_test( ut_MuonSelectorToolsTester_MC
   SCRIPT test/ut_MuonSelectorToolsTester_MC.sh
   PROPERTIES TIMEOUT 450
   POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
