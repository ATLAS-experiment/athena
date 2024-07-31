#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single piplus 1GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422047.ParticleGun_single_piplus_Pt1.merge.EVNT.e7967_e5984_tid20255138_00
# art-input-nfiles: 1
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: SiHitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml 
# art-output: dcube*
# art-html: dcube_shifter_last

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
relname="r24"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_piplus_1GeV_simreco_${relname}.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/RDOAnalysis_piplus_1GeV_simreco_${relname}.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/nightly_references/2024-06-01T2101/physval_piplus1GeV_simreco_2024-06-01T2101.root

script=test_MC_mu0_simreco.sh

echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}
