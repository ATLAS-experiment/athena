#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, 10 GeV Electrons, no pileup
# art-input: mc21_14TeV.900494.PG_single_epm_Pt10_etaFlatnp0_43.evgen.EVNT.e8481
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
relname="r25.0.39"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/HitValid_run4_ele10GeV_simreco.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/RDOAnalysis_run4_ele10GeV_simreco.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_run4_ele10GeV_simreco.root

script=test_MC_Run4_mu0_simreco.sh
echo "Executing script ${script}"
echo " "
"$script" ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec} 10000

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
