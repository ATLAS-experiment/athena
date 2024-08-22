#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, no pileup
# art-input: mc15_14TeV:mc15_14TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.evgen.EVNT.e8357
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

ref_21p9=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/601237_ttbar_allhad_PU0_ITk_21p9p26_v1.IDPVM.root

script=test_MC_Run4_mu0_simreco.sh
echo "Executing script ${script}"
echo " "
"$script" ${ArtInFile} ${ref_21p9}
