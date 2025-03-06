#!/bin/bash
# art-description: Generate TV input files
# art-type: grid
# art-include: main/Athena
# art-input-nfiles: 2
# art-output: TVinput_*.root


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
    FPGATrackSim_F100.sh -o "${LABEL}.root" --ttbar --events 3
ls -ltr
mv dataprep.root "TVinput_${LABEL}.root"

LABEL="F100_singleMu_region0"
run "${LABEL}" \
    FPGATrackSim_F100.sh -o "${LABEL}.root" --single-muon --events 10
ls -ltr
mv dataprep.root "TVinput_${LABEL}.root"

LABEL="F600_ttbar"
run "${LABEL}" \
    FPGATrackSim_F600.sh -o "${LABEL}.root" --ttbar --events 3
ls -ltr
mv test.root "TVinput_${LABEL}.root"

LABEL="F600_singleMu"
run "${LABEL}" \
    FPGATrackSim_F600.sh -o "${LABEL}.root" --single-muon --events 3
ls -ltr
mv test.root "TVinput_${LABEL}.root"