#!/bin/bash
#
# art-description: Run MC20e pile-up pre-mixing with 2018 geometry and conditions, 25ns pile-up, MT output containers
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 22.0-mc20/Athena
# art-include: 23.0/Athena
# art-include: main/Athena
# art-output: mc20e_SP.RDO.pool.root
# art-output: mc20e_MP_fork_evt0.RDO.pool.root
# art-output: mc20e_MP_fork_evt1.RDO.pool.root

export ATHENA_CORE_NUMBER=8

DigiOutFileNameSP="mc20e_SP.RDO.pool.root"
DigiOutFileNameMP0="mc20e_MP_fork_evt0.RDO.pool.root"
DigiOutFileNameMP1="mc20e_MP_fork_evt1.RDO.pool.root"

InputHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/*"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_MC)")

Digi_tf.py \
    --CA \
    --inputHITSFile ${InputHitsFile} \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 38 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameSP} \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'all:Campaigns.MC20e' \
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
    --CA \
    --multiprocess --athenaMPEventsBeforeFork 0 \
    --inputHITSFile ${InputHitsFile} \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 38 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameMP0} \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'all:Campaigns.MC20e' \
    --skipEvents 0

rc2=$?
if [ $status -eq 0 ]; then
  status=$rc2
fi
echo "art-result: $rc2 Digi_tf.py MP fork after 0"

Digi_tf.py \
    --CA \
    --multiprocess --athenaMPEventsBeforeFork 1 \
    --inputHITSFile ${InputHitsFile} \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 38 \
    --maxEvents 25 \
    --outputRDOFile ${DigiOutFileNameMP1} \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'all:Campaigns.MC20e' \
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
        --ignore-leaves McEventCollection_p5_TruthEvent.m_genParticles.m_m index_ref
    rc4=$?
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
        --ignore-leaves McEventCollection_p5_TruthEvent.m_genParticles.m_m index_ref
    rc5=$?
    if [[ $status -eq 0 ]]; then
        status=$rc5
    fi
fi
echo "art-result: $rc5 SP vs MP fork after 1"

exit $status
