#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single muon 1GeV
# art-type: grid
# art-input: mc23_13p6TeV:mc23_13p6TeV.902070.PG_singlemuon_Pt1_etaFlat0_2p7.recon.RDO.e8582_e8528_s4162_s4114_r15704
# art-input-nfiles: 10
# art-cores: 8
# art-memory: 4096
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml 
# art-output: art_core_0
# art-output: dcube*
# art-html: dcube_shifter_last

relname="r25.0.26"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeRef=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_mu1GeV_reco.root 

script=test_MC_mu0_reco.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${ArtInFile} ${dcubeRef}
