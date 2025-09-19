#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single mu 10GeV 
# art-type: grid
# art-input: mc23_13p6TeV:mc23_13p6TeV.902073.PG_singlemuon_Pt10_etaFlat0_2p7.merge.EVNT.e8582_e8528
# art-input-nfiles: 1
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: HitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml 
# art-output: dcube*
# art-html: dcube_shifter_last

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
relname="r25.0.39"
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/HitValid_mu_10GeV_simreco.root
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/RDOAnalysis_mu_10GeV_simreco.root 
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_mu10GeV_simreco.root

script=test_MC_mu0_simreco_multicores.sh

echo "Executing script ${script}"
echo " "
"$script" ${ArtProcess} ${ArtInFile} ${dcuberef_sim} ${dcuberef_rdo} ${dcuberef_rec}

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
