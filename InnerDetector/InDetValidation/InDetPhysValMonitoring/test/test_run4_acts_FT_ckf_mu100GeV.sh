#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, Single muon 100GeV, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: idpvm*.root
# art-output: last_results/idpvm*.root
# art-output: last_results/art_download_AtlasBuildStamp
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ckf_last

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
ref_idpvm_athena=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_run4_mu100GeV_reco_r25.root

script=test_MC_Run4_acts_FT_ckf_mu0_reco.sh
echo "Executing script ${script}"
echo " "
"$script" ${rdo} ${ref_idpvm_athena}
