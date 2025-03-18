#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using 21.0-compatible geometry and FullG4MT_QS
# art-type: build
# art-include: 24.0/Athena
# art-include: 24.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation

# MC16 setup
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
export TRF_ECHO=1
Sim_tf.py \
    --CA \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --simulator 'FullG4MT_QS' \
    --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --DataRunNumber 284500 \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "Hits.pool.root" \
    --maxEvents 2

rc=$?
echo  "art-result: $rc simulation"
exit  $rc
