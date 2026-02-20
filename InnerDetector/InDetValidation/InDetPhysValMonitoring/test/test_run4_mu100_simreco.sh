#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, 100 GeV Muons, no pileup
# art-type: grid
# art-input: mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.evgen.EVNT.e8481
# art-input-nfiles: 10
# art-cores: 8
# art-memory: 4096
# art-include: main/Athena
# art-output: physval*.root
# art-output: HitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
relname="r25.0.49"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/HitValid_run4_mu100GeV_simreco.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/RDOAnalysis_run4_mu100GeV_simreco.root
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_run4_mu100GeV_simreco.root

script=test_MC_Run4_mu0_simreco_multicores.sh
echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${ArtInFile} ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec} 1000

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
