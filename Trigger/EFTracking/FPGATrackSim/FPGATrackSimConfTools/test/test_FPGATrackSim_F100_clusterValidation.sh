#!/bin/bash
# art-description: Compare F100 to C100 on Zee pu0 events (full detector)
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_compare

set -e
PREFIX_F100="F100"
PREFIX_C100="C100"
INPUT_AOD_FILE_F100="xAOD_${PREFIX_F100}.root"

DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/dcube/config/FPGATrackSimIDTPMconfigClusterValidation.xml"
IDTPM_CONFIG_F100="FPGATrackSimConfTools/IDTPM_clusterValidationF100.json"
IDTPM_CONFIG_C100="FPGATrackSimConfTools/IDTPM_clusterValidationC100.json"
# Don't run if dcube config for nightly cmp is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 $DCUBE_CONFIG not found"
    exit 1
fi

# Don't run if IDTPM config for nightly is not found
if [ -z "$IDTPM_CONFIG_F100" ]; then
    echo "IDTPM config $IDTPM_CONFIG_F100 not found"
    exit 1
fi

if [ -z "$IDTPM_CONFIG_C100" ]; then
    echo "IDTPM config $IDTPM_CONFIG_C100 not found"
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

# Run F100 and produce IDTPM output
run "${PREFIX_F100} pipeline" \
    FPGATrackSim_F100.sh -o $INPUT_AOD_FILE_F100 -t -n 2 -c -q

get_files -data "$IDTPM_CONFIG_F100"
run "IDTPM" \
    runIDTPM.py --inputFileNames=$INPUT_AOD_FILE_F100 \
                --outputFilePrefix="IDTPM.${PREFIX_F100}" \
                --trkAnaCfgFile="$IDTPM_CONFIG_F100"

# Run C100 and produce IDTPM output
run "${PREFIX_C100} pipeline" \
    source FPGATrackSim_CommonEnv.sh -t -n 2 -c
    python -m FPGATrackSimConfTools.C100Config \
        --evtMax="${RDO_EVT_ANALYSIS}" \
        --filesInput="${RDO_ANALYSIS}" \
        Output.AODFileName="xAOD_${PREFIX_C100}.root" \
        PhysVal.IDTPM.trkAnaCfgFile=$IDTPM_CONFIG_C100 \
        PhysVal.IDTPM.outputFilePrefix="IDTPM.${PREFIX_C100}_temp" \
        Trigger.FPGATrackSim.writeClustersToAOD="$WRITE_XAOD_CLUSTERS"

get_files -data "$IDTPM_CONFIG_C100"
run "IDTPM" \
    runIDTPM.py --inputFileNames="xAOD_${PREFIX_C100}.root" \
                --outputFilePrefix="IDTPM.${PREFIX_C100}" \
                --trkAnaCfgFile="$IDTPM_CONFIG_C100"


# Run dcube
run "dcube-F100_vs_C100" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_compare \
        --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        -M "${PREFIX_F100}" \
        -R "${PREFIX_C100}" \
        -r IDTPM.${PREFIX_C100}.HIST.root \
        IDTPM.${PREFIX_F100}.HIST.root