#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS and legacy athena, electron events with pt=100 GeV
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last,dcube_athena_shifter_last,dcube_athena_acts_comparison_shifter

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900497.PG_single_epm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675664._000001.pool.root.1

script=test_run4_acts_ckf_epm_reco.sh
echo "Executing script ${script} "
bash ${script} ${rdo} 
