################################################################################
# Package: TauAnalysisTools
################################################################################

# Declare the package name:
atlas_subdir( TauAnalysisTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )
find_package( onnxruntime )

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
   atlas_add_library( TauAnalysisToolsLib${CMODE_SUFFIX}
      TauAnalysisTools/*.h Root/*.cxx
      PUBLIC_HEADERS TauAnalysisTools
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} 
                     AsgTools xAODEgamma xAODMuon xAODJet xAODTau xAODTracking 
                     xAODTruth TruthUtils PATCoreLib PATInterfaces 
                     AsgAnalysisInterfaces
                     xAODCore xAODBTagging AthOnnxInterfaces AthOnnxUtilsLib ColumnarCoreLib ColumnarModeLib${CMODE_SUFFIX} ColumnarTauLib ColumnarEventInfoLib
                     ColumnarTruthLib ColumnarJetLib TruthUtils
      PRIVATE_LINK_LIBRARIES AthLinks xAODEventInfo xAODMetaData PathResolver )

   if( NOT XAOD_STANDALONE )
      atlas_add_component( TauAnalysisTools${CMODE_SUFFIX}
         src/*.cxx src/components/*.cxx
         INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
         LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AthenaBaseComps AsgTools xAODTau GaudiKernel
         TauAnalysisToolsLib${CMODE_SUFFIX} xAODCore xAODBTagging AthOnnxInterfaces AthOnnxUtilsLib ColumnarCoreLib ColumnarTauLib ColumnarEventInfoLib )
   else()
      atlas_add_component( TauAnalysisTools${CMODE_SUFFIX}
         src/components/*.cxx
         LINK_LIBRARIES TauAnalysisToolsLib${CMODE_SUFFIX})
   endif()

endforeach()

atlas_add_dictionary( TauAnalysisToolsDict
   TauAnalysisTools/TauAnalysisToolsDict.h
   TauAnalysisTools/selection.xml
   LINK_LIBRARIES TauAnalysisToolsLib )

atlas_add_dictionary( TauAnalysisToolsEnumDict
   TauAnalysisTools/Enums.h
   TauAnalysisTools/selection-enums.xml )

# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( TauAnalysisToolsExample
      util/TauAnalysisToolsExample.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODCore xAODEventInfo
      xAODTau AsgTools PileupReweightingLib TauAnalysisToolsLib ColumnarCoreLib ColumnarTauLib ColumnarEventInfoLib )
endif()

# Test(s) in the package:
if( NOT XAOD_STANDALONE )
   atlas_add_test( ut_TauAnalysisTools_test
      SOURCES test/ut_TauAnalysisTools_test.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthAnalysisBaseCompsLib POOLRootAccessLib
      GaudiKernel AsgTools xAODTau TauAnalysisToolsLib CxxUtils ColumnarCoreLib ColumnarTauLib ColumnarEventInfoLib
      POST_EXEC_SCRIPT noerror.sh )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py )
