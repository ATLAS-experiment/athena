#!/bin/sh
#
# art-description: CA-based config Pile-up Pre-tracking for MC23a running serial
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: PU_TRK.RDO.pool.root
# art-architecture: '#x86_64-intel'

events=50

RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"
RDO_PU_File="PU_TRK.RDO.pool.root"

Reco_tf.py \
  --CA \
  --inputRDOFile ${RDO_BKG_File} \
  --outputRDO_PUFile ${RDO_PU_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --preInclude 'Campaigns.MC23a' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --conditionsTag 'OFLCOND-MC23-SDR-RUN3-01'  \
  --geometryVersion 'ATLAS-R3S-2021-03-02-00' \
  --preExec="flags.Tracking.doBackTracking=False;" \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --athenaopts "all:--threads=1" \
  --imf False

pretracking=$?
echo  "art-result: $pretracking PUTracking"
status=$pretracking

# Regression
reg=-9999
if [ ${pretracking} -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root --file=${RDO_PU_File}
    reg=$?
    status=$reg
fi

echo  "art-result: $reg regression"

exit $status
