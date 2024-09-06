#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single pi 1GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422047.ParticleGun_single_piplus_Pt1GeV_Rel22073
# art-input-nfiles: 10
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

relname="r24.0.61"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_piplus1GeV_reco.root

script=test_MC_mu0_reco.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${ArtInFile} ${dcubeRef}
