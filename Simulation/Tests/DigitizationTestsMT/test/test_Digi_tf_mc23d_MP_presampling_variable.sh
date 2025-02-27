#!/bin/bash
#
# art-description: Run a digitization example to compare configuration between ConfGetter and the new ComponentAccumulator approach.
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc23d_presampling.SingleBS.RDO.pool.root
# art-output: log.*
# art-output: DigiPUConfig*

export ATHENA_CORE_NUMBER=8

if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

Events=50
DigiOutFileName="mc23d_presampling.SingleBS.RDO.pool.root"
HSHitsFile="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc21/HITS/mc21_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8453_s3864/HITS.29241942._001453.pool.root.1" # TODO update?
HighPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4154_s4120/*"
LowPtMinbiasHitsFiles1="${ATLAS_REFERENCE_DATA}/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4154_s4120/*"

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Digi_tf.py \
    --CA \
    --multiprocess \
    --PileUpPresampling True \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --inputHITSFile ${HSHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles1} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles1} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileName} \
    --postExec 'HITtoRDO:cfg.getService("PileUpEventLoopMgr").AllowSerialAndMPToDiffer=False' \
    --postInclude 'all:PyJobTransforms.UseFrontier' 'HITtoRDO:DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC23d' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

rc1=-9999
if [ $status -eq 0 ]; then
    mv ${DigiOutFileName} backup_${DigiOutFileName}
    rm PoolFileCatalog.xml
    RDOMerge_tf.py \
        --CA \
        --PileUpPresampling True \
        --inputRDOFile backup_${DigiOutFileName} \
        --outputRDO_MRGFile ${DigiOutFileName} \
        --postInclude "default:PyJobTransforms.UseFrontier" "all:PyJobTransforms.SortInput"
    rc1=$?
    rm backup_${DigiOutFileName}
    status=$rc1
fi
echo "art-result: $rc1 RDOMerge_tf.py"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc1 -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --order-trees --no-diff-meta "$DigiOutFileName" "${ATLAS_REFERENCE_DATA}/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName" --diff-root
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

rc6=-9999
if [[ $rc1 -eq 0 ]]
then
    art.py compare grid --entries 10 "$1" "$2" --mode=semi-detailed --order-trees --file="$DigiOutFileName" --diff-root
    rc6=$?
    status=$rc6
fi
echo "art-result: $rc6 regression"

exit $status
