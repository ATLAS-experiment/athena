#!/bin/sh
#
# art-description: MC23a-style simulation using FullG4 (13 TeV ttbar input - needs updating)
# art-type: build
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-08
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 2

rc=$?
echo  "art-result: $rc simulation"
exit $rc
