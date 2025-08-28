# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PMGTools )

# Extra libraries based on the build environment:
set( xaod_access_lib )
set( extra_private_libs )
# ... for AnalysisBase
if( XAOD_STANDALONE )
   set( xaod_access_lib xAODRootAccess )
# ... for AthAnalysisBase (Athena calls this POOLRootAccess)
else()
   set( xaod_access_lib POOLRootAccessLib )
   set( extra_private_libs AthAnalysisBaseCompsLib )
endif()

# External(s) used by the package:
find_package( ROOT COMPONENTS Core Hist RIO )

# Libraries in the package:
atlas_add_library( PMGToolsLib
   PMGTools/*.h Root/*.cxx
   PUBLIC_HEADERS PMGTools
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools PATCoreLib PATInterfaces PMGAnalysisInterfacesLib xAODTruth
   PRIVATE_LINK_LIBRARIES FourMomUtils PathResolver TruthUtils xAODEventInfo xAODJet xAODMetaData ${extra_private_libs})

atlas_add_component( PMGTools
   src/components/*.cxx
   LINK_LIBRARIES PMGToolsLib )

atlas_add_dictionary( PMGToolsDict
   PMGTools/PMGToolsDict.h
   PMGTools/selection.xml
   LINK_LIBRARIES PMGToolsLib )

# Executable(s) in the package:
#
# ... for Athena/AthAnalysis
if( NOT XAOD_STANDALONE )
   atlas_add_executable( MyPMGApp
      test/MyPMGApp.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AsgTools GaudiKernel PATInterfaces
      PMGAnalysisInterfacesLib PMGToolsLib xAODJet xAODTruth ${xaod_access_lib} )
endif()

# Python module
atlas_install_python_modules( python/*.py )

# Install helper utility
atlas_install_runtime( util/generator-weights.py )

# Test(s) in the package:
#
# ... for AnalysisBase
if( XAOD_STANDALONE )
   atlas_add_test( ut_PMGSherpa22VJetsWeightTool_test
      SOURCES test/ut_PMGSherpa22VJetsWeightTool_test.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} PMGToolsLib ${xaod_access_lib}
      POST_EXEC_SCRIPT nopost.sh )

   atlas_add_test( ut_PMGSherpaVjetsSysTool_test
      SOURCES test/ut_PMGSherpaVjetsSysTool_test.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} PMGToolsLib ${xaod_access_lib}
      POST_EXEC_SCRIPT nopost.sh )
endif()
# ... AthAnalysis/AnalysisBase
# Athena tests only with POOL access (ATEAM-681), AnalysisBase tests with xAOD access
atlas_add_test( ut_PMGTruthWeightTool_test
   SOURCES test/ut_PMGTruthWeightTool_test.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AsgTools PMGAnalysisInterfacesLib PMGToolsLib xAODTruth ${xaod_access_lib}
   POST_EXEC_SCRIPT nopost.sh )

# Run tests:
#atlas_add_test( WeightHelpersTest
#                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/testWeightHelpers.py
#                POST_EXEC_SCRIPT nopost.sh )
