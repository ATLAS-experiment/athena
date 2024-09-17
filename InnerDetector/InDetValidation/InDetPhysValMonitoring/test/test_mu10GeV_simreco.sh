#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single mu 10GeV 
# art-type: grid
# art-input: mc23_13p6TeV:mc23_13p6TeV.902073.PG_singlemuon_Pt10_etaFlat0_2p7.merge.EVNT.e8582_e8528
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
relname="r24"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_mu_100GeV_simreco_${relname}.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/RDOAnalysis_mu_100GeV_simreco_${relname}.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/nightly_references/2024-06-01T2101/physval_mu100GeV_simreco_2024-06-01T2101.root 

script=test_MC_mu0_simreco.sh

echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}
