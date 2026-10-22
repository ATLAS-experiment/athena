################################################################################
# Package: MuonMomentumCorrections
################################################################################

# Declare the package name:
atlas_subdir( MuonMomentumCorrections )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree RIO Physics )
find_package( GTest )

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
   atlas_add_library( MuonMomentumCorrectionsLib${CMODE_SUFFIX}
      MuonMomentumCorrections/*.h Root/*.cxx
      PUBLIC_HEADERS MuonMomentumCorrections
      INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES  ${ROOT_LIBRARIES} AsgTools ColumnarMuonLib MuonAnalysisInterfacesLib FourMomUtils ColumnarCoreLib ColumnarModeLib${CMODE_SUFFIX} ColumnarTrackingLib ColumnarEventInfoLib ColumnarVariantLib
      PATInterfaces AsgDataHandlesLib AsgMessagingLib
      PRIVATE_LINK_LIBRARIES xAODEventInfo PathResolver )

   if( NOT XAOD_STANDALONE )
      atlas_add_component( MuonMomentumCorrections${CMODE_SUFFIX}
         src/*.h src/*.cxx src/components/*.cxx
         INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
         LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps xAODMuon xAODCore MuonAnalysisInterfacesLib  FourMomUtils
         xAODBase GaudiKernel MuonSelectorToolsLib MuonMomentumCorrectionsLib${CMODE_SUFFIX} StoreGateLib AsgMessagingLib PathResolver TruthUtils)
   else()
      atlas_add_component( MuonMomentumCorrections${CMODE_SUFFIX}
         src/components/*.cxx
         LINK_LIBRARIES MuonMomentumCorrectionsLib${CMODE_SUFFIX})
   endif()

   atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
      SOURCES test/gt_ColumnarToolTests.cxx
      INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib MuonMomentumCorrectionsLib${CMODE_SUFFIX}
      POST_EXEC_SCRIPT noerror.sh )

endforeach()

atlas_add_dictionary( MuonMomentumCorrectionsDict
   MuonMomentumCorrections/MuonMomentumCorrectionsDict.h
   MuonMomentumCorrections/selection.xml
   LINK_LIBRARIES MuonMomentumCorrectionsLib )

# Executable(s) in the package:
atlas_add_executable( MCAST_Tester
   util/MCAST_Tester.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess
   xAODEventInfo xAODMuon xAODCore PATInterfaces xAODCore AsgTools AsgAnalysisInterfaces FourMomUtils
   MuonSelectorToolsLib MuonMomentumCorrectionsLib  MuonAnalysisInterfacesLib AsgMessagingLib  PathResolver)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

