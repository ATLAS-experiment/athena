#!/bin/bash
# art-description: Test running F200 pipeline VS offline
# art-type: grid
# art-include: main/Athena
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_cmp


ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
INPUT_AOD_FILE="xAOD_F200.root"
PIPELINE_SCRIPT="${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/FPGATrackSimWorkflow/FPGATrackSim_F200.sh"
IDTPM_PREFIX="IDTPM.F200"
IDTPM_CONFIG="InDetTrackPerfMon/F200_singleMu_region0.json"
DCUBE_CONFIG_FILE="EFvsOfflDcube.xml"
CONV_TXT_FILE="OfflToEFCnv.txt"

CONV_TXT=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $CONV_TXT_FILE -print -quit 2>/dev/null )
DCUBE_CONFIG=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $DCUBE_CONFIG_FILE -print -quit 2>/dev/null )

# Don't run if dcube config is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 DCube config $DCUBE_CONFIG not found"
    exit 1
fi

# Don't run if conversion config is not found
if [ -z "$CONV_TXT" ]; then
    echo "art-result: 1 Conversion config $CONV_TXT not found"
    exit 1
fi

# Don't run if pipeline sceript is not found
if [ -z "$PIPELINE_SCRIPT" ]; then
    echo "art-result: 1 Pipeline script $PIPELINE_SCRIPT not found"
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
    $PIPELINE_SCRIPT $INPUT_AOD_FILE

run "IDTPM" \
    runIDTPM.py --inputFileNames=$INPUT_AOD_FILE \
                --outputFilePrefix=$IDTPM_PREFIX \
                --writeAOD_IDTPM \
                --trkAnaCfgFile=$IDTPM_CONFIG 

run "Offline to EF conversion" \
    IDTPMcnv.py -i $IDTPM_PREFIX.HIST.root -c ${CONV_TXT} -o ref

run "dcube-F200-latest" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_cmp \
        --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        -M "F200" \
        -R "C000" \
        -r ${IDTPM_PREFIX}.HIST.ref.root \
        ${IDTPM_PREFIX}.HIST.root
