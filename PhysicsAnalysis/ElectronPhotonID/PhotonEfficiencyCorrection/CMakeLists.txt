# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( PhotonEfficiencyCorrection )

# External(s) for the package:
find_package( ROOT COMPONENTS Core )

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

   # Libraries in the package:
   atlas_add_library( PhotonEfficiencyCorrectionLib${CMODE_SUFFIX}
      PhotonEfficiencyCorrection/*.h Root/*.cxx
      PUBLIC_HEADERS PhotonEfficiencyCorrection
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES EgammaAnalysisInterfacesLib AsgTools xAODEgamma
      PATCoreAcceptLib PATInterfaces AsgMessagingLib ElectronEfficiencyCorrectionLib${CMODE_SUFFIX}
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} xAODEventInfo PathResolver AsgMessagingLib )

   if( NOT XAOD_STANDALONE )
      atlas_add_component( PhotonEfficiencyCorrection${CMODE_SUFFIX}
         src/*.h src/*.cxx src/components/*.cxx
         LINK_LIBRARIES AsgTools AthenaBaseComps EgammaAnalysisInterfacesLib
         ElectronEfficiencyCorrectionLib${CMODE_SUFFIX} GaudiKernel PATCoreAcceptLib PATInterfaces
         xAODCore xAODEgamma xAODEventInfo PhotonEfficiencyCorrectionLib${CMODE_SUFFIX} )
   else()
      atlas_add_component( PhotonEfficiencyCorrection${CMODE_SUFFIX}
         src/components/*.cxx
         LINK_LIBRARIES PhotonEfficiencyCorrectionLib${CMODE_SUFFIX} )
   endif()

   atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
                  SOURCES test/gt_ColumnarToolTests.cxx
                  LINK_LIBRARIES ColumnarTestFixturesLib${CMODE_SUFFIX} PhotonEfficiencyCorrectionLib${CMODE_SUFFIX}
                  PRIVATE_WORKING_DIRECTORY
                  POST_EXEC_SCRIPT noerror.sh )

endforeach()

atlas_add_dictionary( PhotonEfficiencyCorrectionDict
   PhotonEfficiencyCorrection/PhotonEfficiencyCorrectionDict.h
   PhotonEfficiencyCorrection/selection.xml
   LINK_LIBRARIES PhotonEfficiencyCorrectionLib )

# Executable(s) in the package:
atlas_add_executable( PrintPhotonSF
   util/PrintPhotonSF.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ElectronEfficiencyCorrectionLib PATCoreAcceptLib
   PhotonEfficiencyCorrectionLib )

if( XAOD_STANDALONE AND NOT COLUMNAR_ANALYSIS )
   atlas_add_executable( TestxAODPhotonAlg
      util/TestxAODPhotonAlg.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools EgammaAnalysisInterfacesLib
      ElectronEfficiencyCorrectionLib PATCoreAcceptLib PATInterfaces PathResolver
      xAODCore xAODEgamma xAODEventInfo PhotonEfficiencyCorrectionLib AsgMessagingLib )

   #Tests
   atlas_add_test(ut_RunOnASGAOD
	          SCRIPT TestxAODPhotonAlg $ASG_TEST_FILE_MC
		  POST_EXEC_SCRIPT noerror.sh)
   

endif()

