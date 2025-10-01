#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS and legacy athena, electron events with pt=10 GeV
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last

rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900494.PG_single_epm_Pt10_etaFlatnp0_43.recon.RDO.e8481_s4494_r16632/RDO.45451600._000028.pool.root.1

script=test_run4_acts_ckf_epm_reco.sh
echo "Executing script ${script} "
bash ${script} ${rdo} 

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
