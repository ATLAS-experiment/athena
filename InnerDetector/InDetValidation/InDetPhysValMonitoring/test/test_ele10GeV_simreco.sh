#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single ele 10GeV 
# art-type: grid
# art-input: mc23_13p6TeV:mc23_13p6TeV.902079.PG_singleelectron_Pt10_etaFlat0_2p5.merge.EVNT.e8582_e8528
# art-input-nfiles: 10
# art-cores: 8
# art-memory: 4096
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: HitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml 
# art-output: dcube*
# art-html: dcube_shifter_last

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
relname="r24.0.109"
dcuberef_sim=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/HitValid_ele10GeV_simreco.root
dcuberef_rdo=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/RDOAnalysis_ele10GeV_simreco.root
dcuberef_rec=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_ele10GeV_simreco.root

script=test_MC_mu0_simreco_multicores.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${ArtInFile} ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}

