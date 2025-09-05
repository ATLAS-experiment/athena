#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single piplus 5GeV
# art-type: grid
# art-input: mc23_13p6TeV:mc23_13p6TeV.902084.PG_singlepi_Pt5_etaFlat0_2p5.merge.EVNT.e8582_e8528
# art-input-nfiles: 1
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
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/HitValid_piplus_5GeV_simreco.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/RDOAnalysis_piplus_5GeV_simreco.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_piplus5GeV_simreco.root

script=test_MC_mu0_simreco.sh

echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}
