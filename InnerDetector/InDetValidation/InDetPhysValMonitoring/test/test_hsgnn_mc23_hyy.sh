#!/bin/bash
# art-description: HSGNN test for mc23 ttbar MC
# art-type: grid
# art-input: mc23_13p6TeV.602421.PhPy8EG_PDF4LHC21_ggH_NNLOPS_gammagamma.merge.AOD.e8559_e8528_s4162_s4114_r14622_r14663
# art-input-nfiles: 1
# art-memory: 4096
# art-include: main/Athena
# art-output: idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-output: DAOD_PHYSVAL*
# art-html: dcube_shifter_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

script=test_HSGNN.sh

echo "Executing script ${script}"
echo " "
"$script"

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
