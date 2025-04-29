#!/bin/bash
#
# art-description: Run 4 pile-up ttbar MP tests
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: main/Athena
# art-output: RUN4_ttbar_SP.RDO.pool.root
# art-output: RUN4_ttbar_MP_fork_evt0.RDO.pool.root
# art-output: RUN4_ttbar_MP_fork_evt1.RDO.pool.root

export ATHENA_CORE_NUMBER=4

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

DigiOutFileNameSP="RUN4_ttbar_SP.RDO.pool.root"
DigiOutFileNameMP0="RUN4_ttbar_MP_fork_evt0.RDO.pool.root"
DigiOutFileNameMP1="RUN4_ttbar_MP_fork_evt1.RDO.pool.root"

Events=50
HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4150/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4150/*"

geotag="ATLAS-P2-RUN4-03-00-00"
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Digi_tf.py \
--CA \
--conditionsTag default:${conditions} \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:${geotag} \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameSP} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc Digi_tf.py SP"

rc1=-9999
if [ $status -eq 0 ]; then
    mv ${DigiOutFileNameSP} backup_${DigiOutFileNameSP}
    rm PoolFileCatalog.xml
    RDOMerge_tf.py --CA --inputRDOFile backup_${DigiOutFileNameSP} --outputRDO_MRGFile ${DigiOutFileNameSP}
    rc1=$?
    rm backup_${DigiOutFileNameSP}
    status=$rc1
fi
echo "art-result: $rc1 RDOMerge_tf.py SP"

Digi_tf.py \
--multiprocess --athenaMPEventsBeforeFork 0 \
--CA \
--conditionsTag default:${conditions} \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:${geotag} \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameMP0} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc2=$?
if [ $status -eq 0 ]; then
  status=$rc2
fi
echo "art-result: $rc2 Digi_tf.py MP fork after 0"

Digi_tf.py \
--multiprocess --athenaMPEventsBeforeFork 1 \
--CA \
--conditionsTag default:${conditions} \
--digiSeedOffset1 170 --digiSeedOffset2 170 \
--digiSteeringConf 'StandardSignalOnlyTruth' \
--geometryVersion default:${geotag} \
--inputHITSFile ${HSHitsFile} \
--inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
--inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
--jobNumber 568 \
--maxEvents ${Events} \
--outputRDOFile ${DigiOutFileNameMP1} \
--preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
--postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
--postInclude 'PyJobTransforms.UseFrontier' \
--skipEvents 0

rc3=$?
if [[ $status -eq 0 ]]; then
    status=$rc3
fi
echo "art-result: $rc3 Digi_tf.py MP fork after 1"

rc4=-9999
if [[ $rc1 -eq 0 ]] && [[ $rc2 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP0} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves index_ref xAOD::AuxContainerBase_InTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_InTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_InTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_InTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_OutOfTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_OutOfTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_OutOfTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_OutOfTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt
    rc4=$?
    # Other than index_ref the listed leaves all have nan values. These leaves are not valid for truth jets, so should not have been written to the input HITS files. This should go away once the inputs are updated.
    if [[ $status -eq 0 ]]; then
        status=$rc4
    fi
fi
echo "art-result: $rc4 SP vs MP fork after 0"

rc5=-9999
if [[ $rc1 -eq 0 ]] && [[ $rc3 -eq 0 ]]
then
    acmd.py diff-root ${DigiOutFileNameSP} ${DigiOutFileNameMP1} \
        --mode=semi-detailed --error-mode resilient --order-trees \
        --ignore-leaves index_ref xAOD::AuxContainerBase_InTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_InTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_InTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_InTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_OutOfTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_OutOfTimeAntiKt4TruthJetsAuxDyn.HadronConeExclTruthLabelPt xAOD::AuxContainerBase_OutOfTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelLxy xAOD::AuxContainerBase_OutOfTimeAntiKt6TruthJetsAuxDyn.HadronConeExclTruthLabelPt
    rc5=$?
    # Other than index_ref the listed leaves all have nan values. These leaves are not valid for truth jets, so should not have been written to the input HITS files. This should go away once the inputs are updated.
    if [[ $status -eq 0 ]]; then
        status=$rc5
    fi
fi
echo "art-result: $rc5 SP vs MP fork after 1"

exit $status
