#!/bin/sh
#
# art-description: MC23-style RUN3 simulation using FullG4MT_QS in AthenaMT
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

export ATHENA_CORE_NUMBER=8

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-08
Sim_tf.py \
    --CA \
    --multithreaded \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
    --simulator 'FullG4MT_QS' \
    --postExec 'all:from IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002"))' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc22_pre/valid2.900311.Epos_minbias_inelastic_lowjetphoton.evgen.EVNT.e8480/EVNT.30415957._001017.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 200 \
    --postExec 'EVNTtoHITS:with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --order-trees --diff-root --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

rc3=-9999
if [ $rc -eq 0 ]
then
    FilterHit_tf.py \
        --CA \
        --inputHITSFile="test.CA.HITS.pool.root" \
        --maxEvents 200 \
        --postInclude 'PyJobTransforms.UseFrontier' \
        --skipEvents="0" \
        --TruthReductionScheme="SingleGenParticle" \
        --outputHITS_FILTFile="filt.CA.HITS.pool.root" \
        --postExec 'with open("ConfigFiltCA.pkl", "wb") as f: cfg.store(f)' \
        --imf False
    rc3=$?
    if [ $status -eq 0 ]
    then
        status=$rc3
    fi
    mv log.FilterHitTf log.FilterHitTf.CA
fi
echo  "art-result: $rc3 filtCA"

rc4=-9999
if [ $rc3 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}  --order-trees --mode=semi-detailed --diff-root --file=filt.CA.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 filt_regression"
exit $status
