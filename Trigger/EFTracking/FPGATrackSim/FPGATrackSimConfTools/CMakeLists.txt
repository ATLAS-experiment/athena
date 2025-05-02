	# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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
atlas_install_python_modules( python/*.py python/helperScripts/*.py )
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
                PROPERTIES TIMEOUT 450)

atlas_add_test( FPGATrackSimBankGeneration_test 
                SCRIPT FPGATrackSimBankGeneration.sh
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 450)

atlas_add_test( FPGATrackSim_F600_test
                PRIVATE_WORKING_DIRECTORY 
                SCRIPT FPGATrackSim_F600.sh -t -n 1 -s 8
                LOG_IGNORE_PATTERN ".*ERROR Propagation reached the step count limit.*|.*ERROR Propagation failed: PropagatorError:2.*"
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 600 )

atlas_add_test( FPGATrackSim_F410_test
                SCRIPT FPGATrackSim_F410.sh -t -n 1 -s 8
                PRIVATE_WORKING_DIRECTORY
                LOG_IGNORE_PATTERN ".*ERROR Propagation reached the step count limit.*|.*ERROR Propagation failed: PropagatorError:2.*|.*Found error reading file at line:.*" # ACTS (C)KF related errors
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 600 )

atlas_add_test( FPGATrackSim_F610_test
                PRIVATE_WORKING_DIRECTORY 
                SCRIPT FPGATrackSim_F610.sh -m -n -1
                LOG_IGNORE_PATTERN ".*ERROR Propagation reached the step count limit.*|.*ERROR Propagation failed: PropagatorError:2.*|.* WARNING FPE.*FPGATrackSimProtoTackFitAlg.*"
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 600 )

atlas_add_test( FPGATrackSimHelperFunctions
                PRIVATE_WORKING_DIRECTORY 
                SCRIPT python -m FPGATrackSimConfTools.FPGATrackSimHelperFunctions
                POST_EXEC_SCRIPT noerror.sh 
                PROPERTIES TIMEOUT 5 )
