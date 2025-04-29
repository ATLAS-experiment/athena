#!/bin/sh
#
# art-description: CA-based config Pile-up Pre-tracking for MC23a
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: log.*
# art-output: *.pkl
# art-output: *.txt
# art-output: PU_TRK.RDO.pool.root
# art-architecture: '#x86_64-intel'

events=50

export ATHENA_CORE_NUMBER=8

RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"
RDO_PU_File="PU_TRK.RDO.pool.root"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Reco_tf.py \
  --CA \
  --multithreaded True \
  --inputRDOFile ${RDO_BKG_File} \
  --outputRDO_PUFile ${RDO_PU_File} \
  --maxEvents ${events} \
  --skipEvents 0 \
  --preInclude 'Campaigns.MC23a' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --conditionsTag "default:${conditions}" \
  --geometryVersion "default:${geometry}" \
  --preExec="flags.Tracking.doBackTracking=False;" \
  --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False

pretracking=$?
echo  "art-result: $pretracking PUTracking"
status=$pretracking

rdomerge=-9999

# RDOMerge
if [ $pretracking -eq 0 ]
then
    mv ${RDO_PU_File} backup_${RDO_PU_File}
    rm PoolFileCatalog.xml
    RDOMerge_tf.py \
      --CA \
      --PileUpPresampling True \
      --inputRDOFile backup_${RDO_PU_File}\
      --outputRDO_MRGFile ${RDO_PU_File}\
      --postInclude "default:PyJobTransforms.UseFrontier" "all:PyJobTransforms.SortInput"
      rdomerge=$?
      rm backup_${RDO_PU_File}
      status=$rdomerge
fi

echo  "art-result: $rdomerge RDOmerge"

# Regression test
reg=-9999
if [ ${rdomerge} -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 4 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --diff-root --file=${RDO_PU_File}
    reg=$?
    status=$reg
fi

echo  "art-result: $reg regression"

exit $status
