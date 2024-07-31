#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single piplus 5GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422048.ParticleGun_single_piplus_Pt5.merge.EVNT.e7967_e5984_tid20255154_00
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
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_piplus5GeV_simreco_2024-05-20T2101.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/RDOAnalysis_piplus_5GeV_simreco_r24.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/physval_test_piplus5GeV_simreco_r24.0.49.root

script=test_MC_mu0_simreco.sh

echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}
