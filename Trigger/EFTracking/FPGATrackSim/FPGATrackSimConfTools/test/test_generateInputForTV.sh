#!/bin/bash
# art-description: Generate TV input files
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-input: mc21_14TeV:mc21_14TeV.601190.PhPy8EG_AZNLO_Zmumu.recon.RDO.e8481_s4203_r14697
# art-input-nfiles: 11
# art-output: TVinput_*.root

echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList

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

LABEL="F100_ttbar_fullDetector"
run "${LABEL}" \
    FPGATrackSim_F100.sh -o "${LABEL}.root" --ttbar --events 1 
ls -ltr
mv dataprep.root "TVinput_${LABEL}.root"

LABEL="F100_Zmumu_fullDetector"
run "${LABEL}" \
    FPGATrackSim_F100.sh -o "${LABEL}.root" --single-muon --events 10 -i $fileList
ls -ltr
mv dataprep.root "TVinput_${LABEL}.root"

LABEL="F600_Zmumu_phiSlice"
run "${LABEL}" \
    FPGATrackSim_F600.sh -o "${LABEL}.root" --single-muon --events 100 -i $fileList
ls -ltr
mv test.root "TVinput_${LABEL}.root"

LABEL="F610_Zmumu_phiSlice"
run "${LABEL}" \
    FPGATrackSim_F610.sh -o "${LABEL}.root" --single-muon --events 100 -i $fileList
ls -ltr
mv test.root "TVinput_${LABEL}.root"