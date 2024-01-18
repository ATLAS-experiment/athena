# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the Athena project
# --> README.md before you modify this file
#

#################################################################################
# General
#################################################################################
atlas_add_citest( DuplicateClass
   SCRIPT python -c 'import ROOT'
   PROPERTIES FAIL_REGULAR_EXPRESSION "class .* is already in" )

#################################################################################
# Digitization/Simulation
#################################################################################

atlas_add_citest( FastChain
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/FastChain.sh )

atlas_add_citest( SimulationRun3AF3
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w AF3 --threads 4 -e '--maxEvents 20' --run-only
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( SimulationRun3AF3Checks
   SCRIPT RunWorkflowTests_Run3.py --CI -s -w AF3 --checks-only --output-path ../SimulationRun3AF3
   LOG_IGNORE_PATTERN "WARNING FPE"  # ignore FPEs from Geant4
   DEPENDS_SUCCESS SimulationRun3AF3 )

atlas_add_citest( SimulationRun4FullSim
   SCRIPT RunWorkflowTests_Run4.py --CI -s -w FullSim -e '--maxEvents 5' --no-output-checks
   LOG_IGNORE_PATTERN "WARNING FPE" )  # ignore FPEs from Geant4

atlas_add_citest( PileUpPresamplingRun2
   SCRIPT RunWorkflowTests_Run2.py --CI -p -w PileUpPresampling -e '--maxEvents 5 --conditionsTag OFLCOND-MC16-SDR-RUN2-11' --no-output-checks )

atlas_add_citest( PileUpPresamplingRun3
   SCRIPT RunWorkflowTests_Run3.py --CI -p -w PileUpPresampling -e '--maxEvents 5' --no-output-checks )

atlas_add_citest( OverlayRun2MC
   SCRIPT RunWorkflowTests_Run2.py --CI -o -w MCOverlay -e '--CA True --conditionsTag OFLCOND-MC16-SDR-RUN2-11' )

atlas_add_citest( OverlayRun2MC_Legacy
   SCRIPT RunWorkflowTests_Run2.py --CI -o -w MCOverlay -e '--CA False --conditionsTag OFLCOND-MC16-SDR-RUN2-11' )

atlas_add_citest( OverlayRun2Data
   SCRIPT RunWorkflowTests_Run2.py --CI -o -w DataOverlay -e '--conditionsTag CONDBR2-BLKPA-RUN2-11' )

atlas_add_citest( OverlayRun3MC
   SCRIPT RunWorkflowTests_Run3.py --CI -o -w MCOverlay -e '--CA True' )

atlas_add_citest( OverlayRun3MC_Legacy
   SCRIPT RunWorkflowTests_Run3.py --CI -o -w MCOverlay -e '--CA False' )

#################################################################################
# Standard reconstruction workflows
#################################################################################

atlas_add_citest( RecoRun2Data
   SCRIPT RunWorkflowTests_Run2.py --CI -r -w DataReco -e '--CA True --maxEvents 25 --conditionsTag CONDBR2-BLKPA-RUN2-11 --preExec pass' )

atlas_add_citest( RecoRun2MC
   SCRIPT RunWorkflowTests_Run2.py --CI -r -w MCReco --threads 0 -e '--CA "all:True" "RDOtoRDOTrigger:False" --conditionsTag "default:OFLCOND-MC16-SDR-RUN2-11" "RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02" --maxEvents 25' )

atlas_add_citest( RecoRun2MC_PileUp
   SCRIPT RunWorkflowTests_Run2.py --CI -p -w MCPileUpReco -e '--CA True --maxEvents 5 --inputRDO_BKGFile=../../PileUpPresamplingRun2/run_d1730/myRDO.pool.root --conditionsTag OFLCOND-MC16-SDR-RUN2-11 --preExec="flags.Exec.FPE=500;" ' --no-output-checks  # go two levels up as the test runs in a subfolder
   DEPENDS_SUCCESS PileUpPresamplingRun2 )

atlas_add_citest( RecoRun3Data
   SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a q449 --threads 8 -e '--CA True --maxEvents 100 --preExec="flags.Exec.FPE=500;"' --run-only
   PROPERTIES PROCESSORS 8 )

atlas_add_citest( RecoRun3Data_Checks
   SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a q449 --checks-only --output-path ../RecoRun3Data
   DEPENDS_SUCCESS RecoRun3Data )

atlas_add_citest( RecoRun3Data_Bulk
    SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a f1333 --threads 8  -e ' --CA True --skipEvents 100 --maxEvents 500 --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data'  --run-only --no-output-checks
   PROPERTIES PROCESSORS 8 )

atlas_add_citest( RecoRun3Data_Bulk_Checks
    SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a f1333 --checks-only --output-path ../RecoRun3Data_Bulk --no-output-checks
   DEPENDS_SUCCESS RecoRun3Data_Bulk )

atlas_add_citest( RecoRun3Data_Express
    SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a x785 -e ' --CA True --maxEvents 25 --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00428353.express_express.merge.RAW._lb0800._SFO-ALL._0001.1  ' --no-output-checks
    LOG_IGNORE_PATTERN "WARNING FPE .*PixelChargeLUTCalibCondAlg"
    # ignore FPEs from PixelChargeLUTCalibCondAlg
  )

atlas_add_citest( RecoRun3Data_Cosmics
   SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a q450 -e '--CA True --maxEvents 25  --preExec="all:flags.Exec.FPE=500;"'  --no-output-checks )

atlas_add_citest( RecoRun3Data_Calib
   SCRIPT RunWorkflowTests_Run3.py --CI -r -w DataReco -a q451 -e '--CA True --maxEvents 25  --preExec="all:flags.Exec.FPE=500;"' --no-output-checks )

atlas_add_citest( RecoRun3MC
   SCRIPT RunWorkflowTests_Run3.py --CI -r -w MCReco -e '--CA "all:True" "RDOtoRDOTrigger:False" --maxEvents 25' )

atlas_add_citest( RecoRun3MC_PileUp
    SCRIPT RunWorkflowTests_Run3.py --CI -p -w MCPileUpReco -e '--CA "all:True" "RDOtoRDOTrigger:False" --maxEvents 5 --inputRDO_BKGFile=../../PileUpPresamplingRun3/run_d1760/myRDO.pool.root --preExec="flags.Exec.FPE=500;" ' --no-output-checks  # go two levels up as the test runs in a subfolder
   DEPENDS_SUCCESS PileUpPresamplingRun3 )

atlas_add_citest( RecoRun4MC
   SCRIPT RunWorkflowTests_Run4.py --CI -r -w MCReco -e '--maxEvents 5 --inputHITSFile=../../SimulationRun4FullSim/run_s3761/myHITS.pool.root' --no-output-checks  # go two levels up as the test runs in a subfolder
   DEPENDS_SUCCESS SimulationRun4FullSim )

#################################################################################
# Standard Derivation  workflows
#################################################################################

atlas_add_citest( DerivationRun2Data_PHYS
   SCRIPT RunWorkflowTests_Run2.py --CI -d -w Derivation --tag data_PHYS --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun2Data_PHYSLITE
   SCRIPT RunWorkflowTests_Run2.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun2MC_PHYS
   SCRIPT RunWorkflowTests_Run2.py --CI -d -w Derivation --tag mc_PHYS --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun2MC_PHYSLITE
   SCRIPT RunWorkflowTests_Run2.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun3Data_PHYS
   SCRIPT RunWorkflowTests_Run3.py --CI -d -w Derivation --tag data_PHYS --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun3Data_PHYSLITE
   SCRIPT RunWorkflowTests_Run3.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun3MC_PHYS
   SCRIPT RunWorkflowTests_Run3.py --CI -d -w Derivation --tag mc_PHYS --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( DerivationRun3MC_PHYSLITE
   SCRIPT RunWorkflowTests_Run3.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4
   PROPERTIES PROCESSORS 4 )

atlas_add_citest( RecoRun4MC_DAODPHYS
   SCRIPT RunWorkflowTests_Run4.py --CI -d -w Derivation -e '--maxEvents 5 --inputAODFile=../../RecoRun4MC/run_q447/myAOD.pool.root' --no-output-checks  # go two levels up as the test runs in a subfolder
   DEPENDS_SUCCESS RecoRun4MC )

#################################################################################
# Analysis
#################################################################################

atlas_add_citest( CPAlgorithmsRun2MC_PHYS
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type mc --no-physlite-broken --force-input ../DerivationRun2MC_PHYS/run_mc_PHYS_Run2/DAOD_PHYS.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun2MC_PHYS )

atlas_add_citest( CPAlgorithmsRun2MC_PHYSLITE
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type mc --physlite --no-physlite-broken --force-input ../DerivationRun2MC_PHYSLITE/run_mc_PHYSLITE_Run2/DAOD_PHYSLITE.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun2MC_PHYSLITE )

atlas_add_citest( CPAlgorithmsRun2Data_PHYS
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type data --no-physlite-broken --force-input ../DerivationRun2Data_PHYS/run_data_PHYS_Run2/DAOD_PHYS.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun2Data_PHYS )

atlas_add_citest( CPAlgorithmsRun2Data_PHYSLITE
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type data --physlite --no-physlite-broken --force-input ../DerivationRun2Data_PHYSLITE/run_data_PHYSLITE_Run2/DAOD_PHYSLITE.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun2Data_PHYSLITE )

atlas_add_citest( CPAlgorithmsRun3MC_PHYS
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type mc --no-physlite-broken --force-input ../DerivationRun3MC_PHYS/run_mc_PHYS_Run3/DAOD_PHYS.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun3MC_PHYS )

atlas_add_citest( CPAlgorithmsRun3MC_PHYSLITE
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type mc --physlite --no-physlite-broken --force-input ../DerivationRun3MC_PHYSLITE/run_mc_PHYSLITE_Run3/DAOD_PHYSLITE.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun3MC_PHYSLITE )

atlas_add_citest( CPAlgorithmsRun3Data_PHYS
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type data --no-physlite-broken --force-input ../DerivationRun3Data_PHYS/run_data_PHYS_Run3/DAOD_PHYS.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun3Data_PHYS )

atlas_add_citest( CPAlgorithmsRun3Data_PHYSLITE
   SCRIPT athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py - --block-config --data-type data --physlite --no-physlite-broken --force-input ../DerivationRun3Data_PHYSLITE/run_data_PHYSLITE_Run3/DAOD_PHYSLITE.myOutput.pool.root
   DEPENDS_SUCCESS DerivationRun3Data_PHYSLITE )


#################################################################################
# Data Quality
#################################################################################

atlas_add_citest( DataQuality_Run3MC
   SCRIPT Run3DQTestingDriver.py 'Input.Files=["../RecoRun3MC/run_q445/myAOD.pool.root"]' DQ.Environment=AOD DQ.Steering.doHLTMon=False --threads=1
   DEPENDS_SUCCESS RecoRun3MC )

atlas_add_citest( DataQuality_Run3Data_Postprocessing
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/DataQuality_Run3Data_Postprocessing.sh
   DEPENDS_SUCCESS RecoRun3Data )

atlas_add_citest( DataQuality_Run3Data_AODtoHIST
   SCRIPT Reco_tf.py --AMI=q449 --CA --inputAODFile="../RecoRun3Data/run_q449/myAOD.pool.root" --outputHISTFile=DataQuality_Run3Data_AODtoHIST.root   --preExec="all:flags.Exec.FPE=500;" --athenaopts='--threads=1'
   DEPENDS_SUCCESS RecoRun3Data )

#################################################################################
# e/gamma
#################################################################################

# Test the egamma ART chain
atlas_add_citest( EgammaART
   SCRIPT ut_egammaARTJob_test.sh )

# Test running egamma from RAW 
atlas_add_citest( EgammaRAW
   SCRIPT ut_egamma_fromRAW.sh)

# Test running egamma from ESD
atlas_add_citest( EgammaESD
   SCRIPT ut_egamma_fromESD.sh)

#################################################################################
# ACTS
#################################################################################

atlas_add_citest( ACTS_Propagation_ITk
   SCRIPT ActsITkTest.py )

atlas_add_citest( ACTS_Propagation_ID
   SCRIPT ActsExtrapolationAlgTest.py )

atlas_add_citest( ACTS_Workflow
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsWorkflow.sh
   LOG_IGNORE_PATTERN "ActsTrackFindingAlg.*ERROR Propagation reached the step count limit|ActsTrackFindingAlg.*ERROR Propagation failed: PropagatorError:3 Propagation reached the configured maximum number of steps with the initial parameters|ActsTrackFindingAlg.*ERROR CombinatorialKalmanFilter failed: CombinatorialKalmanFilterError:5 Propagation reaches max steps before track finding is finished with the initial parameters" )

atlas_add_citest( ACTS_ValidateClusters
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateClusters.sh )

atlas_add_citest( ACTS_ValidateActsCoreSpacePoints
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateActsCoreSpacePoints.sh )

atlas_add_citest( ACTS_ValidateActsTrkSpacePoints
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateActsSpacePoints.sh )

atlas_add_citest( ACTS_ValidateSeeds
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateSeeds.sh )

atlas_add_citest( ACTS_ValidateOrthogonalSeeds 
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateOrthogonalSeeds.sh )

atlas_add_citest( ACTS_ActsPersistifyEDM 
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsPersistifyEDM.sh )

atlas_add_citest( ACTS_ValidateTracks
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateTracks.sh )

atlas_add_citest( ACTS_ValidateResolvedTracks
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateResolvedTracks.sh )

atlas_add_citest( ACTS_ValidateAmbiguityResolution
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsValidateAmbiguityResolution.sh )

atlas_add_citest( ACTS_ActsKfRefitting
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsKfRefitting.sh )

atlas_add_citest( ACTS_ActsGSFRefitting
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsGSFRefitting.sh )

atlas_add_citest( ACTS_ActsGSFInEgamma
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsGSFInEgamma.sh )

atlas_add_citest( ACTS_ActsBenchmarkWithSpot
   SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsBenchmarkWithSpot.sh 8 100
   PROPERTIES PROCESSOR 8
   LOG_IGNORE_PATTERN "ActsTrackFindingAlg.*ERROR Propagation reached the step count limit|ActsTrackFindingAlg.*ERROR Propagation failed: PropagatorError:3 Propagation reached the configured maximum number of steps with the initial parameters|ActsTrackFindingAlg.*ERROR CombinatorialKalmanFilter failed: CombinatorialKalmanFilterError:5 Propagation reaches max steps before track finding is finished with the initial parameters|ActsTrackFindingAlg.Acts.*ERROR.*SurfaceError:1" )

#################################################################################
# Trigger
#################################################################################

atlas_add_citest( TriggerMC
   SCRIPT test_trigAna_RDOtoRDOTrig_v1Dev_build.py )

atlas_add_citest( TriggerData
   SCRIPT test_trig_data_v1Dev_build.py )

atlas_add_citest( TriggerDataCAConfig
   SCRIPT test_trig_data_v1Dev_newJO_build.py )

atlas_add_citest( Trigger_athenaHLT_v1Dev
   SCRIPT test_trigP1_v1Dev_decodeBS_build.py )

atlas_add_citest( Trigger_athenaHLT_v1PhysP1
   SCRIPT test_trigP1_v1PhysP1_build.py )

atlas_add_citest( Trigger_athenaHLT_v1Cosmic
   SCRIPT test_trigP1_v1Cosmic_build.py )
