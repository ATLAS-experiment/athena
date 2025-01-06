# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaPoolExampleAlgorithms )

# Component(s) in the package:
atlas_add_component( AthenaPoolExampleAlgorithms
                     src/*.cxx
                     src/components/*.cxx
		     LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaPoolExampleData AthenaPoolUtilities EventBookkeeperMetaData EventInfo GaudiKernel PersistentDataModel StoreGateLib xAODCore )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Function helping to set up the integration tests
function( _add_test testName toExecute )

   # Look for possible extra arguments:
   cmake_parse_arguments( ARG "" "PRE_EXEC;IGNORE"
      "DEPENDS" ${ARGN} )

   # Create the script that will run the test:
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/test/athenarun_test.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/${testName}_test.sh @ONLY )

   # Helper variable setting extra options on the test:
   set( _options )
   if( ARG_PRE_EXEC )
      list( APPEND _options PRE_EXEC_SCRIPT
         "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_PRE_EXEC} ${testName}" )
   endif()
   if( ARG_IGNORE )
      list( APPEND _options LOG_IGNORE_PATTERN ${ARG_IGNORE} )
   endif()

   # Set up the test:
   atlas_add_test( ${testName}
       SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${testName}_test.sh
       ${_options}
       DEPENDS ${ARG_DEPENDS}
       PROPERTIES TIMEOUT 600 )

endfunction( _add_test )

# Test(s) in the package:
# ---------------------------
set( TEST_IGNORE_STR "Warning in <TFile::Init>: no StreamerInfo found|BYTES_READ|READ_CALLS|DEBUG lookupPFN|DEBUG registered PFN|XMLCatalog +INFO|Found address:|DEBUG.*ThinningCacheTool|DEBUG setAttribute|DEBUG Failed to find TTree:.*to set POOL property|DEBUG.*failed process POOL.*attributes.|metadataItemList|Creating branch for new dynamic|::getBranchInfo|Cache alignment|DEBUG index_ref|DEBUG EventInfo" )

# Write 'Hits', with multistreamand
_add_test( AthenaPoolExample_Write 
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Write.py"
   IGNORE ${TEST_IGNORE_STR}
   PRE_EXEC test/pre_check.sh )

# Read 'Hits' and write 'Tracks'
_add_test( AthenaPoolExample_ReadWrite
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadWrite.py"
   DEPENDS AthenaPoolExample_Write
   IGNORE ${TEST_IGNORE_STR} )

# Read all output
_add_test( AthenaPoolExample_Read
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Read.py"
   DEPENDS AthenaPoolExample_ReadWrite
   IGNORE ${TEST_IGNORE_STR} )

# Copy 'Hits' file without extending provenance
_add_test( AthenaPoolExample_Copy
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Copy.py"
   DEPENDS AthenaPoolExample_Read
   IGNORE ${TEST_IGNORE_STR} )

# Read copied 'Hits' and write 'Tracks'
#_add_test( AthenaPoolExample_ReWriteAgain
#   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReWriteAgainJobOptions.py"
#   DEPENDS AthenaPoolExample_Copy
#   IGNORE ${TEST_IGNORE_STR} )
_add_test( AthenaPoolExample_ReadWriteNext
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadWriteNext.py"
   DEPENDS AthenaPoolExample_Copy
   IGNORE ${TEST_IGNORE_STR} )

# Testing Writing xAOD::ExampleElectrons
_add_test( AthenaPoolExample_WritexAODElectrons
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WritexAODElectrons.py"
   DEPENDS AthenaPoolExample_ReadWriteNext
   IGNORE ${TEST_IGNORE_STR} 
   PRE_EXEC test/pre_check.sh )

# Testing Reading xAOD::ExampleElectrons
_add_test( AthenaPoolExample_ReadxAODElectrons
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadxAODElectrons.py"
   DEPENDS AthenaPoolExample_WritexAODElectrons
   IGNORE ${TEST_IGNORE_STR} )

# Read all
_add_test( AthenaPoolExample_ReadAgain
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadAgain.py"
   DEPENDS AthenaPoolExample_ReadxAODElectrons
   IGNORE ${TEST_IGNORE_STR} )

# Concatenate jobs write 'Hits' and 'Tracks' to different streams
_add_test( AthenaPoolExample_Concat
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Concat.py"
   DEPENDS AthenaPoolExample_ReadAgain
   IGNORE ${TEST_IGNORE_STR}
   PRE_EXEC test/pre_check.sh )
_add_test( AthenaPoolExample_ReadConcat
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Read.py"
   DEPENDS AthenaPoolExample_Concat
   IGNORE ${TEST_IGNORE_STR} )

# Testing 'Conditions' I/O
_add_test( AthenaPoolExample_WriteCond
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WriteCond.py"
   DEPENDS AthenaPoolExample_ReadConcat
   IGNORE ${TEST_IGNORE_STR} )

_add_test( AthenaPoolExample_ReadCond
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadCond.py"
   DEPENDS AthenaPoolExample_WriteCond
   IGNORE ${TEST_IGNORE_STR} )

# Testing 'Metadata' I/O
_add_test( AthenaPoolExample_WriteMeta
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WriteMeta.py"
   DEPENDS AthenaPoolExample_ReadCond
   IGNORE ${TEST_IGNORE_STR}
   PRE_EXEC test/pre_check.sh )
_add_test( AthenaPoolExample_ReadMeta
   "athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadMeta.py"
   DEPENDS AthenaPoolExample_WriteMeta
   IGNORE ${TEST_IGNORE_STR} )


