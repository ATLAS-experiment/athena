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
atlas_install_scripts( test/FPGATrackSimWorkflow/*.sh)
atlas_install_scripts( test/test_FPGATrackSimWorkflow.sh)
atlas_install_scripts( test/test_FPGATrackSimDataPrep.sh)
atlas_install_scripts( test/FPGATrackSimInputTestSetup.sh )

# Tests in the package:
atlas_add_test( FPGATrackSimRegionSlices_test
   SOURCES        test/FPGATrackSimRegionSlices_test.cxx
   INCLUDE_DIRS   ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} FPGATrackSimConfToolsLib )

atlas_add_test( FPGATrackSimConfigFlags_test
                SCRIPT python -m FPGATrackSimConfTools.FPGATrackSimConfigFlags
                POST_EXEC_SCRIPT noerror.sh ) 

atlas_add_test( FPGATrackSimMapGeneration_test 
                SCRIPT FPGATrackSimMapGeneration.sh
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240)

atlas_add_test( FPGATrackSimBankGeneration_test 
                SCRIPT FPGATrackSimBankGeneration.sh
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240)

atlas_add_test( FPGATrackSimFromRDO_test
                SCRIPT FPGATrackSimAnalysisOnRDO.sh
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240 )

atlas_add_test( FPGATrackSimDataPrepOnRDO_test
                SCRIPT FPGATrackSimDataPrepOnRDO.sh
                LOG_IGNORE_PATTERN ".*ERROR.*Global to local transformation did not succeed.*" # TODO remove once we fix strip SP formation
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240 )

atlas_add_test( FPGATrackSimInsideOutOnRDO_test
                SCRIPT FPGATrackSimInsideOutOnRDO.sh
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240 )

atlas_add_test( FPGATrackSim_F200_test
                SCRIPT FPGATrackSim_F200.sh
                LOG_IGNORE_PATTERN ".*ERROR Propagation reached the step count limit.*|.*ERROR Propagation failed: PropagatorError:3.*" # ACTS (C)KF related errors
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 240 )
