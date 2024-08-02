#!/bin/bash
# art-description: Standard test for 2018 data
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/RecJobTransformTests/data18_13TeV/data18_13TeV.00348885.physics_Main.daq.RAW._lb0827._SFO-8._0002.data 
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/nightly_references/2024-06-02T2101/physval_data18_13TeV_1000evt_2024-06-02T2101.root
lastref_dir=last_results


script=test_data_reco.sh

echo "Executing script ${script}"
echo " "
"$script" ${inputBS} ${dcubeRef} ${lastref_dir}
