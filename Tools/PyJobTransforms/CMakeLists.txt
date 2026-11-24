# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PyJobTransforms )

# External dependencies:
find_package( cx_Oracle )
find_package( distro )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-ignore=F401,F821 )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/HelloWorld_tf.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_generic( share/*.db
                       DESTINATION share
                       EXECUTABLE )

# Helper for test definition:
function( _add_test test type ) # optional: extra_error timeout
   if ( ARGC GREATER 2 )
      set( extra_error ${ARGV2} )
   endif()
   if ( ARGC GREATER 3 )
      list( APPEND _props TIMEOUT ${ARGV3} )
   endif()
   atlas_add_test( ${test}
      SCRIPT test/test_${test}.${type}
      PRIVATE_WORKING_DIRECTORY
      # Expected errors
      LOG_IGNORE_PATTERN "An ERROR message|A FATAL error message|message number 11|${extra_error}"
      PROPERTIES ${_props}
      POST_EXEC_SCRIPT noerror.sh )
endfunction( _add_test )

# Tests in the package:
foreach( test Echo_tf Sleep_tf
         transform TransformUtils trfAMI trfArgs trfEnv
         trfExceptions trfExitCodes trfGraph trfLogger
         trfMPTools trfReports trfUtils
         trfArgClasses trfValidateRootFile )
      _add_test( ${test} py )
endforeach()

# Tests with specific extra errors:
_add_test( trfExe py "Fatal error in athena logfile" )
_add_test( trfSignatures py "Output cannot be recognised" )
_add_test( trfJobOptions py "Legacy configuration requested" )
_add_test( trfUtilsDBRelease py "Unable to interpret DBRelease" )
_add_test( trfValidation py "ManagedAthenaTileMon reported|DummyAlg.*An error message number" )

atlas_add_test( trfReportsSignaledExit
   SCRIPT test/test_trfReportsSignaledExit.py
   PRIVATE_WORKING_DIRECTORY
   POST_EXEC_SCRIPT nopost.sh )  # expected to fail

# Tests with longer timeout (mostly for dbg build):
_add_test( LogFileScanner py "" 400 )

if( NOT SIMULATIONBASE AND NOT GENERATIONBASE )
   _add_test( RAWMerge_tf py )
   _add_test( trfSubstepIntegration py )
   _add_test( trfArgClassesATLAS py )
   _add_test( HelloWorldTrf sh "" 600 )
endif()

#The following tests currently fail in main and should be revised
#AODMerge_tf
#AtlasG4_SimTTBar_tf
#ESDMerge_tf
#EVNTMerge_tf
#FullChain_tf
#HISTMerge_tf
#NTUPMerge_tf
#Reco_AthenaMP_tf
#Reco_EOS_tf
#Reco_MC_tf
#Reco_Tier0_tf
#Reco_q222_tf
#Reco_tf
#TAGMerge_tf
#RAWMerge_tf py
