#!/bin/bash
# art-description: Standard test for 2023 data
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

relname="r24.0.67"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_data23_13p6TeV_1000evt.root 
lastref_dir=last_results

script=test_data_reco.sh

conditions="CONDBR2-BLKPA-2023-05"
geotag="ATLAS-R3S-2021-03-02-00"

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${inputBS} ${dcubeRef} ${lastref_dir} ${conditions} ${geotag}
