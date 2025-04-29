#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with Fast Tracking, Single muon 1GeV, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_athena_acts

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900492.PG_single_muonpm_Pt1_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33645151._000047.pool.root.1

script=test_MC_Run4_acts_FT_ckf_mu0_reco.sh
echo "Executing script ${script}"
echo " "
"$script" ${rdo} 1000 --truthMinPt 999
