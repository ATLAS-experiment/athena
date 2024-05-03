	# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrackSimConfTools )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Physics )

# Component(s) in the package:
atlas_add_library( FPGATrackSimConfToolsLib
   src/*.cxx FPGATrackSimConfTools/*.h
   PUBLIC_HEADERS FPGATrackSimConfTools
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib TruthUtils
   LINK_LIBRARIES AthenaBaseComps AtlasHepMCLib GaudiKernel FPGATrackSimObjectsLib PathResolver )

atlas_add_component( FPGATrackSimConfTools
   src/components/*.cxx
   LINK_LIBRARIES FPGATrackSimConfToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( test/FPGATrackSimInputTestSetup.sh )
atlas_install_scripts( test/FPGATrackWorkflow/FPGATrackWrapperFileGeneration.sh
                       test/FPGATrackWorkflow/FPGATrackBankGeneration.sh
                       test/FPGATrackWorkflow/FPGATrackMapGeneration.sh
                       test/FPGATrackWorkflow/FPGATrackConstGeneration.sh
                       test/FPGATrackWorkflow/FPGATrackAnalysisOnWrapper.sh
                       test/FPGATrackWorkflow/FPGATrackAnalysisOnRDO.sh
)

# Tests in the package:
atlas_add_test( FPGATrackSimRegionSlices_test
   SOURCES        test/FPGATrackSimRegionSlices_test.cxx
   INCLUDE_DIRS   ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} FPGATrackSimConfToolsLib )

atlas_add_test( FPGATrackSimConfigFlags_test
                SCRIPT python -m FPGATrackSimConfTools.FPGATrackSimConfigFlags
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( FPGATrackSimWorkflow_WrapperGeneration_test
                SCRIPT FPGATrackWrapperFileGeneration.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)

atlas_add_test( FPGATrackSimWorkflow_MapGeneration_test
                SCRIPT FPGATrackMapGeneration.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)

atlas_add_test( FPGATrackSimWorkflow_BankGeneration_test
                SCRIPT FPGATrackBankGeneration.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)   
                
atlas_add_test( FPGATrackSimWorkflow_ConstGeneration_test
                SCRIPT FPGATrackConstGeneration.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)
                
atlas_add_test( FPGATrackSimWorkflow_WrapperAnalysis_test
                SCRIPT FPGATrackAnalysisOnWrapper.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)
                
atlas_add_test( FPGATrackSimWorkflow_RDOAnalysis_test
                SCRIPT FPGATrackAnalysisOnRDO.sh
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 200)   
