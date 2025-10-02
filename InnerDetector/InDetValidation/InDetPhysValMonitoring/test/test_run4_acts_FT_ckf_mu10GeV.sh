#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, Single muon 100GeV, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: acts-expert-monitoring*.root
# art-output: idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900495.PG_single_muonpm_Pt10_etaFlatnp0_43.recon.RDO.e8481_s4494_r16632/RDO.45451583._000012.pool.root.1

script=test_MC_Run4_acts_FT_ckf_mu0_reco.sh
echo "Executing script ${script}"
echo " "
"$script" ${rdo} 1000

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
