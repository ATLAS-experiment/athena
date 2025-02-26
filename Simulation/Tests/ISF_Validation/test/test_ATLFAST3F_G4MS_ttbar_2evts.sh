#!/bin/sh
#
# art-description: MC23a-style RUN3 simulation using ATLFAST3F_G4MS in serial Athena
# art-type: build
# art-include: 24.0/Athena
# art-include: main/Athena

# RUN3 setup
export TRF_ECHO=1
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
Sim_tf.py \
    --CA \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --simulator 'ATLFAST3F_G4MS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "Hits.pool.root" \
    --maxEvents 2 \
    --jobNumber 1

rc=$?
echo  "art-result: $rc simulation"
exit $rc
