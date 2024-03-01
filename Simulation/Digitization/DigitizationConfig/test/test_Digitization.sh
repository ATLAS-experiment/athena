#!/bin/sh
#
# art-description: Run digitization of an MC16a ttbar sample with 2016a geometry and conditions, without pile-up
# art-type: build
# art-include: 23.0/Athena
# art-include: main/Athena

Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1" \
    --jobNumber 568 \
    --maxEvents 25 \
    --outputRDOFile "mc20a_ttbar.RDO.pool.root" \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --preInclude 'HITtoRDO:Campaigns.MC20a' \
    --skipEvents 0

echo "art-result: $? digitization"
