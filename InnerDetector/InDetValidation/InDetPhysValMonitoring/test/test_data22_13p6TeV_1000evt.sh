#!/bin/bash
# art-description: Standard test for 2022 data
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
inputBS=${artdata}/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_data22_13p6TeV_1000evt.root 

script=test_data_reco.sh

conditions="CONDBR2-BLKPA-2022-15"
geotag="ATLAS-R3S-2021-03-02-00"

echo "Executing script ${script}"
echo " "
"$script" ${inputBS} ${dcubeRef} ${conditions} ${geotag}
