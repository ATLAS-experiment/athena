# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaPoolExampleAlgorithms )

# Component(s) in the package:
atlas_add_component( AthenaPoolExampleAlgorithms
                     src/*.cxx
                     src/components/*.cxx
		             LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaPoolExampleData AthenaPoolUtilities EventBookkeeperMetaData EventInfo GaudiKernel PersistentDataModel StoreGateLib xAODCore )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


# Test(s) in the package:
# ---------------------------
set( TEST_IGNORE_STR "Warning in <TFile::Init>: no StreamerInfo found|BYTES_READ|READ_CALLS|XMLCatalog +INFO|Found address:|DEBUG setAttribute|DEBUG Failed to find TTree:.*to set POOL property|DEBUG.*failed process POOL.*attributes.|metadataItemList|Creating branch for new dynamic|Cache alignment|DEBUG setting xAOD::FileMetaData::productionRelease|Deaccess DbContainer|RootDatabase +DEBUG I/O| Skipping$|^StorageSvc +INFO|INFO DataHeader|DEBUG --->(Reading|Adding) (Assoc|Shape)|DEBUG ---->ClassID:|DB=.*CNT=.*CLID|\\bFID\\b|\\bGUID\\b|index_ref Initial basket size|::getBranchInfo" )

# Write 'Hits', with multistreamand
atlas_add_test( AthenaPoolExample_Write
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Write.py
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR}
   PROPERTIES TIMEOUT 600 )

# Read 'Hits' and write 'Tracks'
atlas_add_test( AthenaPoolExample_ReadWrite
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadWrite.py
   DEPENDS AthenaPoolExample_Write
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Read all output
atlas_add_test( AthenaPoolExample_Read
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Read.py
   DEPENDS AthenaPoolExample_ReadWrite
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Copy 'Hits' file without extending provenance
atlas_add_test( AthenaPoolExample_Copy
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Copy.py
   DEPENDS AthenaPoolExample_Read
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Read copied 'Hits' and write 'Tracks'
#atlas_add_test( AthenaPoolExample_ReWriteAgain
#   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReWriteAgainJobOptions.py
#   DEPENDS AthenaPoolExample_Copy
#   PROPERTIES TIMEOUT 600
#   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

atlas_add_test( AthenaPoolExample_ReadWriteNext
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadWriteNext.py
   DEPENDS AthenaPoolExample_Copy
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Testing Writing xAOD::ExampleElectrons
atlas_add_test( AthenaPoolExample_WritexAODElectrons
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WritexAODElectrons.py
   DEPENDS AthenaPoolExample_ReadWriteNext
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR}
   PROPERTIES TIMEOUT 600 )

# Testing Reading xAOD::ExampleElectrons
atlas_add_test( AthenaPoolExample_ReadxAODElectrons
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadxAODElectrons.py
   DEPENDS AthenaPoolExample_WritexAODElectrons
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Read all
atlas_add_test( AthenaPoolExample_ReadAgain
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadAgain.py
   DEPENDS AthenaPoolExample_ReadxAODElectrons
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Concatenate jobs write 'Hits' and 'Tracks' to different streams
atlas_add_test( AthenaPoolExample_Concat
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Concat.py
   DEPENDS AthenaPoolExample_ReadAgain
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR}
   PROPERTIES TIMEOUT 600 )

atlas_add_test( AthenaPoolExample_ReadConcat
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_Read.py
   DEPENDS AthenaPoolExample_Concat
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Testing 'Conditions' I/O
atlas_add_test( AthenaPoolExample_WriteCond
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WriteCond.py
   DEPENDS AthenaPoolExample_ReadConcat
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

atlas_add_test( AthenaPoolExample_ReadCond
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadCond.py
   DEPENDS AthenaPoolExample_WriteCond
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )

# Testing 'Metadata' I/O
atlas_add_test( AthenaPoolExample_WriteMeta
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_WriteMeta.py
   DEPENDS AthenaPoolExample_ReadCond
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR}
   PROPERTIES TIMEOUT 600 )

atlas_add_test( AthenaPoolExample_ReadMeta
   SCRIPT athena.py AthenaPoolExampleAlgorithms/AthenaPoolExample_ReadMeta.py
   DEPENDS AthenaPoolExample_WriteMeta
   PROPERTIES TIMEOUT 600
   LOG_IGNORE_PATTERN ${TEST_IGNORE_STR} )
