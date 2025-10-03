# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( IsolationSelection )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist Tree TMVA )

set(extra_libs)
if ( NOT XAOD_STANDALONE) 
   set(extra_libs  MuonTesterTreeLib)
endif()
if ( NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS )
  set(extra_libs ${extra_libs} TrkExUtils TrackToCaloLib)
endif()

# Libraries in the package:
atlas_add_library( IsolationSelectionLib
  IsolationSelection/*.h Root/*.cxx
  PUBLIC_HEADERS IsolationSelection
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES AthContainers AsgTools xAODBase xAODEgamma xAODMuon xAODEventInfo xAODPFlow ${extra_libs} AsgDataHandlesLib CxxUtils
  xAODPrimitives PATCoreLib PATInterfaces InDetTrackSelectionToolLib xAODTracking ${ROOT_LIBRARIES} TrackVertexAssociationToolLib InDetTrackSelectionToolLib
  PRIVATE_LINK_LIBRARIES PathResolver FourMomUtils  )

if( NOT XAOD_STANDALONE )
   atlas_add_component( IsolationSelection
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AsgTools AthenaBaseComps IsolationSelectionLib MuonAnalysisInterfacesLib EgammaAnalysisInterfacesLib GaudiKernel FourMomUtils )
else()
   atlas_add_component( IsolationSelection
      src/components/*.cxx
      LINK_LIBRARIES IsolationSelectionLib )
endif()

atlas_add_dictionary( IsolationSelectionDict
   IsolationSelection/IsolationSelectionDict.h
   IsolationSelection/selection.xml
   LINK_LIBRARIES IsolationSelectionLib )

# Executable(s) in the package:
if( XAOD_STANDALONE OR XAOD_ANALYSIS )
   atlas_add_executable( testIsolationSelectionTool
      util/testIsolationSelectionTool.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODEgamma xAODMuon
      xAODPrimitives IsolationSelectionLib xAODRootAccess)
endif()

# Test(s) in the package:
if( XAOD_STANDALONE OR XAOD_ANALYSIS )
   atlas_add_test( ut_reflex SCRIPT test/ut_reflex.py
      POST_EXEC_SCRIPT nopost.sh )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
