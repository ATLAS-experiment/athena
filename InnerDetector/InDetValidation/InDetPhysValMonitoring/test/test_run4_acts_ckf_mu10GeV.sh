#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, Single muon 100GeV, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_shifter_last

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900495.PG_single_muonpm_Pt10_etaFlatnp0_43.recon.RDO.e8481_s4494_r16632/RDO.45451583._000012.pool.root.1
ref_idpvm_athena=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_run4_mu10GeV_reco_r25.root

script=test_MC_Run4_acts_ckf_mu0_reco.sh
echo "Executing script ${script}"
echo " "
"$script" ${rdo} ${ref_idpvm_athena}

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
