# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigDecisionTool )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core )
set( ROOT_LIBRARIES_NOPYTHON ${ROOT_LIBRARIES} )
unset( ROOT_FOUND )
find_package( ROOT COMPONENTS Core PyROOT ROOTTPython )

set( COMMON_LIBRARIES
    ${Boost_LIBRARIES} AsgTools CxxUtils xAODBase xAODTrigger 
    TrigConfHLTData TrigConfL1Data TrigNavStructure TrigRoiConversionLib 
    TrigConfInterfaces TrigDecisionInterface TrigCompositeUtilsLib
    TrigConfHLTUtilsLib AsgDataHandlesLib TrigAnalysisHelpersLib)
set (COMMON_PRIVATE_LIBRARIES
    ${ROOT_LIBRARIES} TrigSteeringEvent TrigConfData)

# Component(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_library( TrigDecisionToolLib
      TrigDecisionTool/*.h Root/*.cxx
      PUBLIC_HEADERS TrigDecisionTool
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${COMMON_LIBRARIES}
      PRIVATE_LINK_LIBRARIES ${COMMON_PRIVATE_LIBRARIES} )
else()
   if( XAOD_ANALYSIS )
      atlas_add_library( TrigDecisionToolLib
         TrigDecisionTool/*.h Root/*.cxx src/*.cxx
         PUBLIC_HEADERS TrigDecisionTool
         INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
         PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
         LINK_LIBRARIES ${COMMON_LIBRARIES} EventInfo GaudiKernel StoreGateLib 
         PRIVATE_LINK_LIBRARIES ${COMMON_PRIVATE_LIBRARIES} AthenaBaseComps AthenaKernel )
   else()
      atlas_add_library( TrigDecisionToolLib
         TrigDecisionTool/*.h Root/*.cxx src/*.cxx
         PUBLIC_HEADERS TrigDecisionTool
         INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
         PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
         LINK_LIBRARIES ${COMMON_LIBRARIES} EventInfo GaudiKernel StoreGateLib
         AnalysisTriggerEvent TrigMuonEvent TrigSteeringEvent TrigNavigationLib TrigDecisionEvent
         PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES_NOPYTHON} AthenaBaseComps
         TrigSteeringEvent AthenaKernel TrigConfData)
   endif()

   atlas_add_component( TrigDecisionTool
      src/components/*.cxx
      LINK_LIBRARIES TrigDecisionToolLib )
endif()

atlas_add_dictionary( TrigDecisionToolDict
   TrigDecisionTool/TrigDecisionToolDict.h
   TrigDecisionTool/selection.xml
   LINK_LIBRARIES TrigDecisionToolLib ${ROOT_LIBRARIES} )

if( NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS )
   atlas_add_test( Templates_test
      SOURCES test/Templates_test.cxx
      LINK_LIBRARIES TrigNavigationLib AthenaKernel GaudiKernel AthContainers
      AnalysisTriggerEvent TrigSteeringEvent TrigMuonEvent
      TrigDecisionToolLib
      POST_EXEC_SCRIPT nopost.sh )
endif()

if (NOT XAOD_ANALYSIS )
   atlas_add_test( Run2TrigDecisionToolOnRAW_test 
      SCRIPT python -m TrigDecisionTool.TrigDecisionToolConfig --RAWRUN2
      POST_EXEC_SCRIPT noerror.sh )

      # AOD test
   atlas_add_test( Run2TrigDecisionToolOnAOD_test
      SCRIPT python -m TrigDecisionTool.TrigDecisionToolConfig
      POST_EXEC_SCRIPT noerror.sh )
endif()

if (NOT XAOD_STANDALONE)
  atlas_add_test( MacroTest
    SCRIPT root -q ${CMAKE_CURRENT_SOURCE_DIR}/test/macroTest.cxx
    PRIVATE_WORKING_DIRECTORY
    POST_EXEC_SCRIPT noerror.sh )
  atlas_add_test( ScriptTest
    SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/scriptTest.py
    PRIVATE_WORKING_DIRECTORY
    POST_EXEC_SCRIPT noerror.sh )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
