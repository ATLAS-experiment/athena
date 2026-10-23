################################################################################
# Package: METUtilities
################################################################################

# Declare the package name:
atlas_subdir( METUtilities )

# Extra dependencies, based on the environment:
set( extra_deps )
if( XAOD_STANDALONE )
   set( extra_deps Control/xAODRootAccess
      Reconstruction/Jet/JetCalibTools )
elseif( XAOD_ANALYSIS )
   set( extra_deps GaudiKernel
        Control/AthenaBaseComps
	PhysicsAnalysis/POOLRootAccess
        Reconstruction/Jet/JetCalibTools )
else()
   set( extra_deps GaudiKernel
        Control/AthenaBaseComps )
endif()

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )
find_package( onnxruntime )
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
   atlas_add_library( METUtilitiesLib${CMODE_SUFFIX}
      METUtilities/*.h Root/*.cxx
      PUBLIC_HEADERS METUtilities
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AsgTools xAODEgamma ColumnarJetLib xAODMissingET
      xAODJet xAODMuon xAODTau xAODTracking InDetTrackSelectionToolLib PATInterfaces
      EgammaAnalysisInterfacesLib MuonAnalysisInterfacesLib JetCalibToolsLib ElectronPhotonFourMomentumCorrectionLib${CMODE_SUFFIX} TauAnalysisToolsLib${CMODE_SUFFIX}
      METInterface AsgDataHandlesLib JetAnalysisInterfacesLib ColumnarMetLib
      PRIVATE_LINK_LIBRARIES EventPrimitives FourMomUtils xAODCore PathResolver )

   if( NOT XAOD_STANDALONE )
      atlas_add_component( METUtilities${CMODE_SUFFIX}
         src/*.h src/*.cxx src/components/*.cxx
         LINK_LIBRARIES GaudiKernel AthenaBaseComps METInterface xAODEgamma
         xAODMuon xAODTau xAODMissingET xAODJet JetCalibToolsLib ElectronPhotonFourMomentumCorrectionLib${CMODE_SUFFIX} MuonSelectorToolsLib TauAnalysisToolsLib${CMODE_SUFFIX}
         TauAnalysisToolsLib${CMODE_SUFFIX} METUtilitiesLib  )
   else()
      atlas_add_component( METUtilities${CMODE_SUFFIX}
         src/components/*.cxx
         LINK_LIBRARIES METUtilitiesLib${CMODE_SUFFIX}  )
   endif()

   atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
      SOURCES test/gt_ColumnarToolTests.cxx
      INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib${CMODE_SUFFIX} METUtilitiesLib${CMODE_SUFFIX}
      POST_EXEC_SCRIPT noerror.sh )

endforeach()

atlas_add_dictionary( METUtilitiesDict
   METUtilities/METUtilitiesDict.h
   METUtilities/selection.xml
   LINK_LIBRARIES METUtilitiesLib )


#set readLib, used below to compile apps/tests
set( readLib )
if( XAOD_STANDALONE )
  set( readLib xAODRootAccess )
else()
  set( readLib POOLRootAccessLib )
endif()

# Executable(s) in the package:
# only compiled in analysis releases
if( XAOD_ANALYSIS )
  # FIX ME: temporarily disabled utilities relying on non-existant code

  # foreach( utility example_METMaker_METSystematicsTool
  #         example_METMaker_advanced example_rebuildTrackMET )
  foreach( utility example_rebuildTrackMET )
    atlas_add_executable( ${utility}
          util/${utility}.cxx
          INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
          LINK_LIBRARIES ${ROOT_LIBRARIES} ${readLib} AsgTools xAODBase
          xAODMissingET xAODCore xAODJet xAODEgamma xAODMuon xAODTau
          JetCalibToolsLib METInterface PATInterfaces METUtilitiesLib )
  endforeach()
endif()


# Test(s) in the package:
if( XAOD_ANALYSIS )
  foreach( test gt_metMaker gt_metSystematicsTool )
    atlas_add_test( ${test}
          SOURCES test/${test}.cxx
          INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
          LINK_LIBRARIES ${ROOT_LIBRARIES} ${GTEST_LIBRARIES} ${readLib}
                AsgTools xAODBase xAODMissingET METUtilitiesLib
          POST_EXEC_SCRIPT noerror.sh )
  endforeach()
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py )

atlas_install_data( data/* )
