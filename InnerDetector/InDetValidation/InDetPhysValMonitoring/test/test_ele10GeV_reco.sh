#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single ele 10GeV
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

#RDO is made at rel 22.0.73
#reference plots are made at rel 22.0.73

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inFile=${artdata}/InDetPhysValMonitoring/inputs/24.0.55/physval.ele10GeV.RDO.root
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/physval_ele10GeV_reco_r24.root

script=test_MC_mu0_reco.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${inFile} ${dcubeRef}
