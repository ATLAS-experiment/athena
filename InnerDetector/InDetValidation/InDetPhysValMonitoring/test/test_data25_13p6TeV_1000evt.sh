#!/bin/bash
# art-description: Standard test for 2025 data
# art-type: grid
# art-memory: 4096
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

relname="r25.0.46"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/RecJobTransformTests/data25_13p6TeV/data25_13p6TeV.00509891.physics_Main.daq.RAW/data25_13p6TeV.00509891.physics_Main.daq.RAW._lb0843._SFO-18._0001.data
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_data25_13p6TeV_1000evt.root

script=test_data_reco.sh

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA)")
geotag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

echo "Executing script ${script}"
echo " "
"$script" ${inputBS} ${dcubeRef} ${conditions} ${geotag}
