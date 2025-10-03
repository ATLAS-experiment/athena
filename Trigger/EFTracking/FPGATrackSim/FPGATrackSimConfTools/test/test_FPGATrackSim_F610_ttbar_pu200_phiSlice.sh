#!/bin/bash
# art-description: Test running F610 pipeline on tbar pu200 FPT
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-input: mc21_14TeV:mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583
# art-input-nfiles: 11
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last


set -e
echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList
PREFIX="F610"
lastref_dir=last_results
INPUT_AOD_FILE="xAOD_${PREFIX}.root"

ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
IDTPM_CONFIG="${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/IDTPM_configs/IDTPM_phiSlice.json"
DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/dcube/config/FPGATrackSimIDTPMconfig.xml"

# Don't run if dcube config for nightly cmp is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 $DCUBE_CONFIG not found"
    exit 1
fi

# Don't run if IDTPM config for nightly is not found
if [ -z "$IDTPM_CONFIG" ]; then
    echo "IDTPM config $IDTPM_CONFIG not found"
    exit 1
fi

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


run "${PREFIX} pipeline" \
    FPGATrackSim_F610.sh -o $INPUT_AOD_FILE -t -n 1000 -q -i $fileList

run "IDTPM" \
    runIDTPM.py --inputFileNames=$INPUT_AOD_FILE \
                --outputFilePrefix="IDTPM.${PREFIX}" \
                --writeAOD_IDTPM \
                --trkAnaCfgFile=$IDTPM_CONFIG

if [ -z $ArtJobType ]; then
    echo "Not in ART environment. Stopping here..."
    echo "IDTPM output: IDTPM.${PREFIX}.HIST.root"
else
art.py download --user=artprod --dst=last_results "$ArtPackage" "$ArtJobName"

run "dcube-${PREFIX}-latest" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_last \
        --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        -M "${PREFIX}" \
        -R "${PREFIX}-previous" \
        -r ${lastref_dir}/IDTPM.${PREFIX}.HIST.root \
        IDTPM.${PREFIX}.HIST.root
fi        