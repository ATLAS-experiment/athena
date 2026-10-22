# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( AssociationUtils )

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

# External(s):
find_package( ROOT COMPONENTS Core )
find_package( Boost COMPONENTS program_options )
find_package( GTest )

foreach (CMODE_NAME CMODE_SUFFIX IN ZIP_LISTS COLUMNAR_MODE_NAME_LIST COLUMNAR_MODE_SUFFIX_LIST)

   # Libraries in the package:
   atlas_add_library( AssociationUtilsLib${CMODE_SUFFIX}
      AssociationUtils/*.h Root/*.cxx
      PUBLIC_HEADERS AssociationUtils
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES AnaAlgorithmLib AsgMessagingLib AsgTools AthContainers AthLinks AsgDataHandlesLib
      xAODBase ColumnarClusterLib ColumnarEventInfoLib ColumnarEgammaLib ColumnarJetLib ColumnarMuonLib ColumnarTauLib ColumnarTrackingLib ColumnarCoreLib ColumnarVariantLib xAODMissingET ColumnarModeLib${CMODE_SUFFIX}
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} FourMomUtils )

   if( NOT XAOD_STANDALONE )
      atlas_add_component( AssociationUtils${CMODE_SUFFIX}
         src/*.h src/*.cxx src/components/*.cxx
         LINK_LIBRARIES AthenaBaseComps xAODBase xAODBTagging GaudiKernel AssociationUtilsLib${CMODE_SUFFIX} )
   else()
      atlas_add_component( AssociationUtils${CMODE_SUFFIX}
         src/components/*.cxx
         LINK_LIBRARIES AssociationUtilsLib${CMODE_SUFFIX} )
   endif()

   atlas_add_test( gt_ColumnarToolTests${CMODE_SUFFIX}
      SOURCES test/gt_ColumnarToolTests.cxx
      INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
      LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarTestFixturesLib AssociationUtilsLib${CMODE_SUFFIX}
      POST_EXEC_SCRIPT noerror.sh )

endforeach()

atlas_add_dictionary( AssociationUtilsDict
    AssociationUtils/AssociationUtilsDict.h
    AssociationUtils/selection.xml
    LINK_LIBRARIES AssociationUtilsLib )

if (NOT COLUMNAR_ANALYSIS)

   # Executables in the package:
   if( XAOD_STANDALONE )
      atlas_add_executable( OverlapRemovalTester
         util/OverlapRemovalTester.cxx
         INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
         LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES}
         xAODEventInfo xAODEgamma xAODMuon xAODJet xAODBTagging xAODTau xAODEgamma
         AssociationUtilsLib )
   endif()

   # Tests in the package:
   set( extra_libs )
   if( XAOD_STANDALONE )
      set( extra_libs xAODRootAccess )
   endif()

   # Helper function for defining google tests
   macro( _add_gtest name )
      atlas_add_test( ${name}
         SOURCES test/${name}.cxx
         INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
         LINK_LIBRARIES ${GTEST_LIBRARIES} AsgTools AsgTestingLib AssociationUtilsLib CxxUtils
         ${extra_libs}
         POST_EXEC_SCRIPT noerror.sh )
   endmacro( _add_gtest )

   # Define the google unit tests.
   # Disabling some which don't work in Athena.
   foreach( test gt_AnaToolHandle_test ) # gt_OverlapRemovalInit_test gt_toolbox_test
      _add_gtest( ${test} )
   endforeach()

   if( XAOD_STANDALONE )
      atlas_add_test( ut_assocUtilsDict
         SCRIPT test/ut_assocUtilsDict.py
         POST_EXEC_SCRIPT noerror.sh )
      atlas_add_test( ut_ort_tester
         SCRIPT test/ut_ort_tester.sh
         POST_EXEC_SCRIPT noerror.sh )
   endif()
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
