#!/bin/bash
# art-description: Generate TV input files
# art-type: grid
# art-include: main/Athena
# art-input-nfiles: 2
# art-output: *.txt
# art-output: *.root
# art-output: *.xml


run () {
    name="${1}"
    cmd="${@:2}"
    echo "Running ${name}..."
    time ${cmd}
    rc=$?
    echo "art-result: $rc ${name}"
    if [ $rc != 0 ]; then
        exit $rc
    fi
    return $rc
}

LABEL="F100_ttbar_wholeDetector"
run "${LABEL}" \
    FPGATrackSimDataPrepOnRDO.sh --ttbar --events 3
mkdir -p "${LABEL}"
mv dataprep.root "${LABEL}"/

LABEL="F100_singleMu_region0"
run "${LABEL}" \
    FPGATrackSimDataPrepOnRDO.sh --single-muon --events 10
mkdir -p "${LABEL}"
mv dataprep.root "${LABEL}"/