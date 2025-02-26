#!/bin/sh
#
# art-description: MC23-style RUN3 simulation of monopole samples using FullG4MT_QS
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl
# art-output: G4particle_acceptlist_ExtraParticles.txt.C*
# art-output: PDGTABLE.MeV.C*

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Sim_tf.py \
    --CA \
    --inputEVNTFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc21_13p6TeV.950542.MGPy8EG_DYSpinHalfMonopoles_1gD_1500GeV_valid.merge.EVNT.e8467_e8455.29631249._000005.pool.root.1" \
    --maxEvents="100" \
    --multithreaded="True" \
    --preInclude "EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV,G4DebuggingTools.DebugMonopole" \
    --skipEvents="0" \
    --randomSeed="41" \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --runNumber="950542" \
    --AMITag="s3890" \
    --jobNumber="41" \
    --firstEvent="40001" \
    --outputHITSFile="test.CA.HITS.pool.root" \
    --simulator="FullG4MT_QS" \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf=False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
mv G4particle_acceptlist_ExtraParticles.txt G4particle_acceptlist_ExtraParticles.txt.CA
mv PDGTABLE.MeV PDGTABLE.MeV.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
