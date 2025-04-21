#!/bin/bash
# art-description: Run-4 Sim to DAOD_PHYSVAL and output plots via dcube, on non-all-had ttbar with no pile-up
# art-input: mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-athena-mt: 8

file=test_full_chain_mu0.sh
script="`basename \"$0\"`"
number_of_events=10

echo "Executing script ${file}"
echo " "
"$file" ${number_of_events}

