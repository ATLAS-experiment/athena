# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIEventUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( nlohmann_json )


# Component(s) in the package:
atlas_add_library( HIEventUtilsLib
                   Root/*.cxx
                   PUBLIC_HEADERS HIEventUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools CaloGeoHelpers InDetTrackSelectionToolLib PATCoreAcceptLib xAODForward xAODHIEvent xAODTracking xAODTrigL1Calo xAODCaloEvent
                   PRIVATE_LINK_LIBRARIES AsgMessagingLib CxxUtils nlohmann_json::nlohmann_json PathResolver)

if (NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS)
  atlas_add_component( HIEventUtils
                       src/components/*.cxx src/HITowerWeightTool.cxx src/HIEventFilterAlgRun3.cxx
                       LINK_LIBRARIES AthenaBaseComps GaudiKernel HIEventUtilsLib Navigation AthenaMonitoringKernelLib EventBookkeeperToolsLib
                       PRIVATE_LINK_LIBRARIES PathResolver)
endif ()

atlas_add_dictionary( HIEventUtilsDict
   HIEventUtils/HIEventUtilsDict.h
   HIEventUtils/selection.xml
   LINK_LIBRARIES HIEventUtilsLib )

atlas_install_data(data/*)

if (NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS)
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


atlas_add_test( HIEventSelectionRun3_25OO
                SCRIPT python -m HIEventUtils.HIEventSelectionRun3Config HIPeriodToTest=25OO
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( HIEventSelectionRun3_25NeNe
                SCRIPT python -m HIEventUtils.HIEventSelectionRun3Config HIPeriodToTest=25NeNe
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( HIEventSelectionRun3_25pO
                SCRIPT python -m HIEventUtils.HIEventSelectionRun3Config HIPeriodToTest=25pO
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( HIEventSelectionRun3_23PbPb
                SCRIPT python -m HIEventUtils.HIEventSelectionRun3Config HIPeriodToTest=23PbPb
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )

# this test will be enabled once we have cuts implemented
# atlas_add_test( HIEventSelectionRun3_24PbPb
#                 SCRIPT python -m HIEventUtils.HIEventSelectionRun3Config HIPeriodToTest=24PbPb
#                 PROPERTIES TIMEOUT 300
#                 POST_EXEC_SCRIPT noerror.sh )


endif()
