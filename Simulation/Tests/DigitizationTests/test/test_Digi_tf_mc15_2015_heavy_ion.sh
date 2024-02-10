#!/bin/bash
#
# art-description: Run digitization combining a heavy ion sample produced with MC15 using 2015 geometry and conditions with Zmumu events
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: mc15_2015_heavyIon.RDO.pool.root

DigiOutFileName="mc15_2015_heavyIon.RDO.pool.root"
DigiOutFileNameCA="mc15_2015_heavyIon.CA.RDO.pool.root"
ZmumuHITSFileName="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/mc15_pPb8TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.simul.HITS.e5367_s3164.HITS.11308894._000005.pool.root.1"
HeavyIonHITSFileName="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/mc15_pPb8TeV.420112.Hijing_MinBiasDiff_pPb_8TeV.merge.HITS.e5328_s3148_s3153.HITS.11232927._000249.pool.root.1"

Digi_tf.py \
    --inputHITSFile ${ZmumuHITSFileName} \
    --inputCavernHitsFile ${HeavyIonHITSFileName} \
    --outputRDOFile ${DigiOutFileName} \
    --maxEvents 10 \
    --skipEvents 0 \
    --numberOfCavernBkg 1 \
    --preExec 'all:rec.doHIP.set_Value_and_Lock(True);from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(0.0);jobproperties.Beam.bunchSpacing.set_Value_and_Lock(25);' 'HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags;digitizationFlags.overrideMetadata+=["PhysicsList"];digitizationFlags.TRTRangeCut.set_Value_and_Lock(0.05);' \
    --preInclude 'all:LArConfiguration/LArConfigRun2Old_NoPileup.py' 'HITtoRDO:Digitization/ForceUseOfPileUpTools.py' \
    --postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' 'HITtoRDO:condSeq.TileSamplingFractionCondAlg.G4Version = -1;topSeq+=CfgMgr.JobOptsDumperAlg(FileName="legacyJO.txt")' \
    --postInclude 'all:PyJobTransforms/UseFrontier.py,SimulationJobOptions/postInclude.HijingPars.py,PixelConditionsAlgorithms/postInclude.UsePixelModuleLevelMask.py' \
    --geometryVersion ATLAS-R2-2015-03-01-00 \
    --conditionsTag all:OFLCOND-RUN12-SDR-31-01 \
    --DataRunNumber 295000 \
    --pileupInitialBunch 0 \
    --pileupFinalBunch 0 \
    --bunchSpacing 100 \
    --imf False

rc=$?
status=$rc
echo "art-result: $rc digiOLD"
cp log.HITtoRDO legacy.HITtoRDO

Digi_tf.py \
    --CA \
    --inputHITSFile ${ZmumuHITSFileName} \
    --inputCavernHitsFile ${HeavyIonHITSFileName} \
    --outputRDOFile ${DigiOutFileNameCA} \
    --maxEvents 10 \
    --skipEvents 0 \
    --preInclude 'all:Campaigns.Run2_2015_HeavyIons' \
    --postExec 'HITtoRDO:cfg.getCondAlgo("TileSamplingFractionCondAlg").G4Version=-1;cfg.addEventAlgo(CompFactory.JobOptsDumperAlg(FileName="CAJO.txt"))' \
    --postInclude 'all:PyJobTransforms.UseFrontier,MCTruthSimAlgs.MCTruthSimAlgsConfig.MergeHijingParsCfg,PixelConditionsAlgorithms.PixelConditionsConfig.PostInclude_UsePixelModuleLevelMask' \
    --geometryVersion ATLAS-R2-2015-03-01-00 \
    --conditionsTag all:OFLCOND-RUN12-SDR-31-01 \
    --imf False

rc2=$?
if [[ $status -eq 0 ]]
then
    status=$rc2
fi
echo "art-result: $rc2 digiCA"
cp log.HITtoRDO new.HITtoRDO

rc3=-9999
if [[ $status -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileName} ${DigiOutFileNameCA} \
        --mode=semi-detailed --error-mode resilient --order-trees
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 OLDvsCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

rc5=-9999
if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 regression"

exit $status
