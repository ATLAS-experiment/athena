#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single ele 10GeV 
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422029.ParticleGun_single_ele_Pt10.merge.EVNT.e7967_e5984_tid20255003_00
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
# To be updated once we have those references
relname="r24"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_mu_100GeV_simreco_${relname}.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/RDOAnalysis_mu_100GeV_simreco_${relname}.root
dcuberef_rec=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/physval_ele10GeV_reco_${relname}.root

script=test_MC_mu0_simreco.sh

echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}
