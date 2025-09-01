#!/bin/sh
#
# art-description: MC23-style RUN2 simulation using 21.0-compatible geometry and ATLFAST3F_ACTSMT
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 5999
# art-output: test.CA.HITS.pool.root
# art-output: Config*

# RUN2 setup
geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ignore_pattern="ActsFatras.+ERROR.+No.+start.+volume.+resolved.+Nothing.+left.+to.+do."
Sim_tf.py \
    --CA \
    --conditionsTag "default:${conditions}" \
    --geometryVersion "default:${geometry}" \
    --simulator 'ATLFAST3F_ACTSMT' \
    --postExec 'with open("ConfigCA.pkl", "wb") as f: cfg.store(f)' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationNoIoV' \
    --preExec "flags.Acts.TrackingGeometry.MaterialCalibrationFolder='ACTS/MaterialMaps/ID'; flags.Acts.TrackingGeometry.MaterialSource='material-maps-ATLAS-R2-2016-00-00-00.json'" \
    --ignorePatterns "${ignore_pattern}" \
    --DataRunNumber 284500 \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.CA.HITS.pool.root" \
    --maxEvents 3 \
    --imf False

rc=$?
mv log.EVNTtoHITS log.EVNTtoHITS_CA
echo  "art-result: $rc simCA"
status=$rc

rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 3 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=test.CA.HITS.pool.root
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 regression"

exit $status
