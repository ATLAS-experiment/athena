#!/bin/sh
#
# art-description: MC23-style RUN3 simulation using ATLFAST3MT_QS in serial Athena
# art-type: build
# art-include: 23.0/Athena
# art-include: 24.0/Athena
# art-include: main/Athena

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01
export TRF_ECHO=1
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'ATLFAST3MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "Hits.pool.root" \
    --maxEvents 2 \
    --jobNumber 1

rc=$?
echo  "art-result: $rc simulation"
exit $rc
