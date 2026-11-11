#!/bin/bash
# art-description: Test running pipeline
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-athena-mt: 8
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: geant4validationpipeline/output/websites/tau
# art-output: log.*

set -e
echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList



cp -rL /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/geant4validationpipeline .
R_FILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/geantcheck_25.0.70_tau.root"


run () {
    name="${1}"
    cmd="${@:2}"
    echo "Running ${name}..."
    rc=0
    time ${cmd} || rc=$?
    echo "art-result: $rc ${name}"
    return $rc
}


run "geant4validationpipeline" \
    sh geant4validationpipeline/pipeline.sh --tests tau --maxEvents 10000 --art yes --ref ${R_FILE}
status=$rc
exit $status


