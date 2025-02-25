#!/bin/bash
# art-description: Standard test for 2017 data
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

relname="r25.0.21"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/RecJobTransformTests/data17_13TeV.00324910.physics_Main.daq.RAW._lb0713._SFO-6._0001.data 
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_data17_13TeV_1000evt.root

script=test_data_reco.sh

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
geotag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")

echo "Executing script ${script}"
echo " "
"$script" ${inputBS} ${dcubeRef} ${conditions} ${geotag}
