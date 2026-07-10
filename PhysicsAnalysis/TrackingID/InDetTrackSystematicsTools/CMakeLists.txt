# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the name of the package:
atlas_subdir( InDetTrackSystematicsTools )

# External(s):
find_package( ROOT COMPONENTS Core Hist RIO MathCore )

# Libraries in the package:
atlas_add_library( InDetTrackSystematicsToolsLib
   InDetTrackSystematicsTools/*.h Root/*.cxx
   PUBLIC_HEADERS InDetTrackSystematicsTools
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools PATInterfaces TruthUtils
   xAODEventInfo xAODTracking xAODTruth xAODJet
   PRIVATE_LINK_LIBRARIES FourMomUtils PathResolver )

if( NOT XAOD_STANDALONE )
   atlas_add_component( InDetTrackSystematicsTools
      src/*.h src/*.cxx src/components/*.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AsgTools xAODCore
      xAODTracking xAODJet StoreGateLib AthLinks
      InDetTrackSystematicsToolsLib )
else()
   atlas_add_component( InDetTrackSystematicsTools
      src/components/*.cxx
      LINK_LIBRARIES InDetTrackSystematicsToolsLib )
endif()

atlas_add_dictionary( InDetTrackSystematicsToolsDict
   InDetTrackSystematicsTools/InDetTrackSystematicsToolsDict.h
   InDetTrackSystematicsTools/selection.xml
   LINK_LIBRARIES InDetTrackSystematicsToolsLib )

# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( InDetTrackSystsToolTester
      util/InDetTrackSystsToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODTracking
      xAODCore InDetTrackSystematicsToolsLib )
endif()

atlas_add_executable( graph_to_function
   util/graph_to_function.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if( NOT XAOD_STANDALONE )
   atlas_install_scripts( test/test_TrackSmearingAlg.py test/test_JetTrackFilteringAlg.py )

   atlas_add_test( TrackSmearingAlg
      SCRIPT test_TrackSmearingAlg.py
      POST_EXEC_SCRIPT noerror.sh
      PRIVATE_WORKING_DIRECTORY
      PROPERTIES TIMEOUT 300 )

   atlas_add_test( JetTrackFilteringAlg
      SCRIPT test_JetTrackFilteringAlg.py
      POST_EXEC_SCRIPT noerror.sh
      PRIVATE_WORKING_DIRECTORY
      PROPERTIES TIMEOUT 300 )
endif()

# This will allow us to put files into data/ to test new calibration files
# Uncomment this line after putting test files into data/, then run "make" again to install.
# atlas_install_data( data/*.root )
