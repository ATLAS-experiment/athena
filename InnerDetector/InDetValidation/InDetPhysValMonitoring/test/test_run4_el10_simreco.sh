#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, 10 GeV Electrons, no pileup
# art-input: mc15_14TeV:mc15_14TeV.900035.PG_singleel_Pt10_etaFlatnp0_43.evgen.EVNT.e8185
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

ref_21p9=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/900035_el10_ITk_21p9_v1.IDPVM.root  # Ref release = 21.9.26

script=test_MC_Run4_mu0_simreco.sh
echo "Executing script ${script}"
echo " "
"$script" ${ArtInFile} ${ref_21p9}
