#!/bin/bash
# art-description: Standard test for 2018 data
# art-type: grid
# art-cores: 4
# art-memory: 4096
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: art_core_0
# art-output: dcube*
# art-html: dcube_shifter_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

relname="r24.0.65"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/RecJobTransformTests/data18_13TeV/data18_13TeV.00348885.physics_Main.daq.RAW._lb0827._SFO-8._0002.data 
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_data18_13TeV_1000evt.root 
lastref_dir=last_results


script=test_data_reco_Run2.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${inputBS} ${dcubeRef} ${lastref_dir}
