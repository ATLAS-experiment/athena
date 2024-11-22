#!/bin/bash
# art-description: Test running F100 pipeline
# art-type: grid
# art-include: main/Athena
# art-input-nfiles: 2
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last


set -e

lastref_dir=last_results
INPUT_AOD_FILE="xAOD_F200.root"

ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
IDTPM_CONFIG="${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/IDTPM_configs/F100_singleMu_region0.json"
DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/dcube/config/FPGATrackSimIDTPMconfig.xml"
IDTPM_PREFIX="IDTPM.F200"

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


run "F200 pipeline" \
    FPGATrackSim_F200.sh $INPUT_AOD_FILE

run "IDTPM" \
    runIDTPM.py --inputFileNames=$INPUT_AOD_FILE \
                --outputFilePrefix=$IDTPM_PREFIX \
                --writeAOD_IDTPM \
                --trkAnaCfgFile=$IDTPM_CONFIG \
                --plotsDefFileList="InDetTrackPerfMon/PlotsDefFileList_default.txt" \
                --plotsCommonValuesFile="InDetTrackPerfMon/PlotsDefCommonValues.json"


art.py download --user=artprod --dst=last_results "$ArtPackage" "$ArtJobName"

run "dcube-F200-latest" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_last \
        --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        -M "F200" \
        -R "F200-previous" \
        -r ${lastref_dir}/${IDTPM_PREFIX}.HIST.root \
        ${IDTPM_PREFIX}.HIST.root