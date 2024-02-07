#!/bin/sh
#
# art-description: MC23-style simulation using FullG4_QS (13.6 TeV Zprime input)
# art-type: build
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation

# RUN3 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01
Sim_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --simulator 'FullG4MT_QS' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc23_13p6TeV.800030.Py8EG_A14NNPDF23LO_flatpT_Zprime_Extended.evgen.EVNT.e8514.33116368._004446.pool.root.1' \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 2

rc=$?
echo  "art-result: $rc simulation"
exit $rc
