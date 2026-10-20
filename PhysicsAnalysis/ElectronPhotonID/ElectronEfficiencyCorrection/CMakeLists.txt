# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ElectronEfficiencyCorrection )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Hist MathCore)

if (XAOD_STANDALONE)
  set (extra_private_libs )
else ()
  set (extra_private_libs AthAnalysisBaseCompsLib)
endif ()

# FIX ME: These settings should come from the Project configuration
if (COLUMNAR_ANALYSIS)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeArray;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
elseif (XAOD_STANDALONE)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
else()
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD")
  SET (COLUMNAR_MODE_SUFFIX_LIST "")
endif()

foreach (CMODE_NAME CMODE_SUFFIX IN ZIP_LISTS COLUMNAR_MODE_NAME_LIST COLUMNAR_MODE_SUFFIX_LIST)

   # Component(s) in the package:
   atlas_add_library( ElectronEfficiencyCorrectionLib${CMODE_SUFFIX}
      ElectronEfficiencyCorrection/*.h Root/*.cxx
      PUBLIC_HEADERS ElectronEfficiencyCorrection
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AsgTools AnaAlgorithmLib AsgDataHandlesLib
      ColumnarEgammaLib ColumnarClusterLib ColumnarEventInfoLib PATInterfaces PATCoreAcceptLib AsgMessagingLib EgammaAnalysisInterfacesLib ColumnarModeLib${CMODE_SUFFIX}
      PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${extra_private_libs} xAODEventInfo AsgMessagingLib
      xAODMetaData PathResolver )

   atlas_add_component( ElectronEfficiencyCorrection${CMODE_SUFFIX}
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES ElectronEfficiencyCorrectionLib${CMODE_SUFFIX} )

   atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
                  SOURCES test/gt_ColumnarToolTests.cxx
                  LINK_LIBRARIES ColumnarTestFixturesLib${CMODE_SUFFIX} ElectronEfficiencyCorrectionLib${CMODE_SUFFIX}
                  PRIVATE_WORKING_DIRECTORY
                  POST_EXEC_SCRIPT noerror.sh )

endforeach()

atlas_add_dictionary( ElectronEfficiencyCorrectionDict
   ElectronEfficiencyCorrection/ElectronEfficiencyCorrectionDict.h
   ElectronEfficiencyCorrection/selection.xml
   LINK_LIBRARIES ElectronEfficiencyCorrectionLib )

find_package( Boost COMPONENTS program_options)
# Utilities provided by the package:
atlas_add_executable( EgEfficiencyCorr_mem_check
   util/EgEfficiencyCorr_mem_check.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AsgTools CxxUtils EgammaAnalysisInterfacesLib )

#Test
atlas_add_test(ut_SimpleInitTest
   SCRIPT EgEfficiencyCorr_mem_check
   LOG_IGNORE_PATTERN "Package.AsgTool...\\s+DEBUG|MessageSvc not found")

# AnalysisBase-only utilities:
if ( NOT XAOD_STANDALONE )
   atlas_add_test(ut_testTtool
	          SCRIPT python -m  ElectronEfficiencyCorrection.TElectronTestAlgConfig --filesInput $ASG_TEST_FILE_MC --evtMax=5
              POST_EXEC_SCRIPT noerror.sh
              PROPERTIES TIMEOUT 300)
elseif ( NOT COLUMNAR_ANALYSIS )
   atlas_add_executable( EgEfficiencyCorr_testEgEfficiencyCorr
      util/testEgEfficiencyCorr.cxx util/SFHelpers.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEgamma PATInterfaces
      ElectronEfficiencyCorrectionLib AsgMessagingLib)

   atlas_add_executable( EgEfficiencyCorr_testEgEfficiencyCorrFwd
      util/testEgEfficiencyCorrFwd.cxx util/SFHelpers.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEgamma PATInterfaces
      ElectronEfficiencyCorrectionLib AsgMessagingLib)
    
   atlas_add_executable( EgEfficiencyCorr_testEgEfficiencyCorrWithoutFile
      util/testEgEfficiencyCorrWithoutFile.cxx 
      util/CreateDummyEl.cxx util/SFHelpers.cxx
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ElectronEfficiencyCorrectionLib
      PATInterfaces xAODCaloEvent xAODEgamma xAODEventInfo xAODRootAccess AsgMessagingLib)

   atlas_add_executable( EgEfficiencyCorr_testEgChargeCorr
      util/testEgChargeCorr.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib ElectronEfficiencyCorrectionLib ElectronPhotonSelectorToolsLib 
      PATCoreAcceptLib PATInterfaces xAODCore xAODEgamma xAODEventInfo )

   atlas_add_executable( EgEfficiencyCorr_dumpNPs
      util/dumpNPs.cxx util/CreateDummyEl.cxx
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo AsgMessagingLib
      xAODCore ElectronEfficiencyCorrectionLib )

   #Tests
   atlas_add_test(ut_RunOnASGAOD
	          SCRIPT EgEfficiencyCorr_testEgEfficiencyCorr  $ASG_TEST_FILE_MC 40)
   
   atlas_add_test(ut_TestFunctionality
	          SCRIPT test/testEgEfficiencyDetailed.py
	      	  PROPERTIES TIMEOUT 300)
endif()

atlas_install_python_modules( util/*.py python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
