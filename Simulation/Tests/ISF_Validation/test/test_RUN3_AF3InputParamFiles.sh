#!/bin/sh
#
# art-description: MC21-style simulation using FullG4 for producing input samples needed for the Fast Calorimeter Simulation parametrisation
# art-type: build
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena

# Full chain with special flags
# Deactivated G4Optimizations: MuonFieldOnlyInCalo, NRR, PRR, FrozenShowers
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01

Sim_tf.py \
    --CA True \
    --simulator 'FullG4MT'  \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --postInclude 'all:PyJobTransforms.UseFrontier' 'EVNTtoHITS:ISF_FastCaloSimSD.ISF_FastCaloSimSDToolConfig.PostIncludeParametrizationInputSim_1mm' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV,ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.ISF_FastCaloSimParametrization_SimPreInclude' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc15_13TeV.431004.ParticleGun_pid22_E65536_disj_eta_m25_m20_20_25_zv_0.evgen.EVNT.e6556.EVNT.13283012._000001.pool.root.1" \
    --outputHITSFile "Hits.CA.pool.root" \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --maxEvents=2

rc=$?
status=$rc
echo  "art-result: $rc simCA"

Sim_tf.py \
    --simulator 'FullG4MT'  \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --physicsList 'FTFP_BERT_ATL_VALIDATION' \
    --truthStrategy 'MC15aPlus' \
    --postInclude "all:PyJobTransforms/UseFrontier.py" "EVNTtoHITS:ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_SimPostInclude_1mm.py" \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py,ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_SimPreInclude.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00_VALIDATION' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc15_13TeV.431004.ParticleGun_pid22_E65536_disj_eta_m25_m20_20_25_zv_0.evgen.EVNT.e6556.EVNT.13283012._000001.pool.root.1" \
    --outputHITSFile "Hits.CA.pool.root" \
    --maxEvents=2 \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --simulator 'FullG4MT'  \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --physicsList 'FTFP_BERT_ATL_VALIDATION' \
    --truthStrategy 'MC15aPlus' \
    --postInclude "all:PyJobTransforms/UseFrontier.py" "EVNTtoHITS:ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_SimPostInclude_1mm.py" \
    --preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py,ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_SimPreInclude.py' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00_VALIDATION' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc15_13TeV.431004.ParticleGun_pid22_E65536_disj_eta_m25_m20_20_25_zv_0.evgen.EVNT.e6556.EVNT.13283012._000001.pool.root.1" \
    --outputHITSFile "Hits.pool.root" \
    --maxEvents=2

rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $status -eq 0 ]
then
    # Compare the outputs
    acmd.py diff-root Hits.pool.root Hits.CA.pool.root \
        --error-mode resilient \
        --mode semi-detailed \
        --order-trees
    rc3=$?
    if [ $status -eq 0 ]
    then
        status=$rc3
    fi
fi
echo "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc -eq 0 ]
then
    Reco_tf.py \
        --CA True \
        --inputHITSFile "Hits.CA.pool.root" \
        --outputRDOFile RDO.CA.pool.root \
        --outputESDFile ESD.CA.pool.root \
        --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
        --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
        --preInclude 'all:Campaigns.MC23NoPileUp' \
        --preExec 'all:flags.LAr.ROD.NumberOfCollisions=20;flags.LAr.ROD.UseHighestGainAutoCorr=True;' 'HITtoRDO:flags.Digitization.DoCaloNoise=False' 'RAWtoALL:flags.Reco.EnableTrigger=False' \
        --postInclude 'all:PyJobTransforms.UseFrontier' 'HITtoRDO:ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.PostIncludeISF_FastCaloSimParametrizationDigi' 'RAWtoALL:ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.PostIncludeISF_FastCaloSimParametrizationReco' \
        --maxEvents -1 \
        --autoConfiguration everything
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 reco_simCA"

rc5=-9999
if [ $rc2 -eq 0 ]
then
    Reco_tf.py \
        --inputHITSFile "Hits.pool.root" \
        --outputRDOFile RDO.pool.root \
        --outputESDFile ESD.pool.root \
        --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
        --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
        --DataRunNumber='410000' \
        --postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' 'all:conddb.addOverride("/LAR/BadChannels/BadChannels", "LARBadChannelsBadChannels-MC-empty")' 'all:conddb.addOverride("/TILE/OFL02/STATUS/ADC", "TileOfl02StatusAdc-EmptyBCh")' 'RAWtoALL:StreamESD.ItemList+=["ISF_FCS_Parametrization::FCS_StepInfoCollection#MergedEventSteps","LArHitContainer#*","TileHitVector#*", "TrackRecordCollection#CaloEntryLayer", "TrackRecordCollection#MuonEntryLayer"]' \
        --postInclude "all:PyJobTransforms/UseFrontier.py" "HITtoRDO:ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_DigiTilePostInclude.py" "HITtoRDO:ISF_FastCaloSimParametrization/ISF_FastCaloSimParametrization_DigiPostInclude.py" \
        --preExec "all:rec.Commissioning.set_Value_and_Lock(True);from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(0.0);from LArROD.LArRODFlags import larRODFlags;larRODFlags.NumberOfCollisions.set_Value_and_Lock(20);larRODFlags.nSamples.set_Value_and_Lock(4);larRODFlags.doOFCPileupOptimization.set_Value_and_Lock(True);larRODFlags.firstSample.set_Value_and_Lock(0);larRODFlags.useHighestGainAutoCorr.set_Value_and_Lock(True)" "RAWtoALL:from CaloRec.CaloCellFlags import jobproperties;jobproperties.CaloCellFlags.doLArCellEmMisCalib=False;rec.runUnsupportedLegacyReco=True;rec.doTrigger=False" "HITtoRDO:from Digitization.DigitizationFlags import digitizationFlags;digitizationFlags.doCaloNoise=False" \
        --maxEvents -1 \
        --autoConfiguration everything
    rc5=$?
    if [ $status -eq 0 ]
    then
        status=$rc5
    fi
fi
echo  "art-result: $rc5 reco_simOLD"

diff2=-9999
if [ $status -eq 0 ]
then
    # Compare the outputs
    acmd.py diff-root RDO.pool.root RDO.CA.pool.root \
        --error-mode resilient \
        --mode semi-detailed \
        --order-trees
    diff2=$?
    if [ $status -eq 0 ]
    then
        status=$diff2
    fi
fi
echo "art-result: $diff2 OLDvsCA_RDO"

rc6=-9999
if [ $rc4 -eq 0 ]
then
    FCS_Ntup_tf.py \
        --CA \
        --inputESDFile 'ESD.CA.pool.root' \
        --outputNTUP_FCSFile 'calohit.CA.root' \
        --doG4Hits true
    rc6=$?
    if [ $status -eq 0 ]
    then
        status=$rc6
    fi
fi

echo  "art-result: $rc6 fcs_ntupCA"

rc7=-9999
if [ $rc5 -eq 0 ]
then
    FCS_Ntup_tf.py \
        --inputESDFile 'ESD.pool.root' \
        --outputNTUP_FCSFile 'calohit.root' \
        --doG4Hits true
    rc7=$?
    if [ $status -eq 0 ]
    then
        status=$rc7
    fi
fi

echo  "art-result: $rc7 fcs_ntupOLD"
exit $status
