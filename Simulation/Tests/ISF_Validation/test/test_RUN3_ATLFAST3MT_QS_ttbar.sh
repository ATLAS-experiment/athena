#!/bin/sh
#
# art-description: MC23-style RUN3 simulation using ATLFAST3MT_QS in serial Athena
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 5999
# art-output: test.*.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

unset ATHENA_CORE_NUMBER

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-08
  Sim_tf.py \
      --CA \
      --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
      --simulator 'ATLFAST3MT_QS' \
      --postInclude 'PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
      --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
      --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
      --outputHITSFile "test.CA.HITS.pool.root" \
      --maxEvents 50 \
      --jobNumber 1 \
      --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
      --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

rc4=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi
echo  "art-result: $rc4 regression"

exit $status
