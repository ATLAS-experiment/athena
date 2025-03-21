#!/bin/bash
# art-description: Test running the F600 (InsideOut) pipeline
# art-type: grid
# art-include: main/Athena
# art-input-nfiles: 2
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last


set -e

PREFIX="F600"
lastref_dir=last_results
INPUT_AOD_FILE="xAOD_${PREFIX}.root"

ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
IDTPM_CONFIG="${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/IDTPM_configs/IDTPM_ttbar_region0.json"
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
    source FPGATrackSim_CommonEnv.sh --ttbar -n -1 -q
    python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
        --evtMax=${RDO_EVT_ANALYSIS} \
        --skipEvents=${SKIP_EVENTS} \
        --filesInput=${RDO_ANALYSIS} \
        Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
        Trigger.FPGATrackSim.bankDir=${BANKS_5L} \
        Trigger.FPGATrackSim.runCKF=$RUN_CKF \
        Trigger.FPGATrackSim.region=0 \
        Trigger.FPGATrackSim.Hough.genScan=True \
        Trigger.FPGATrackSim.spacePoints=False \
        Trigger.FPGATrackSim.tracking=False \
        Trigger.FPGATrackSim.sampleType="${SAMPLE_TYPE}" \
        Trigger.FPGATrackSim.doEDMConversion=True \
        Trigger.FPGATrackSim.doOverlapRemoval=True \
        Trigger.FPGATrackSim.Hough.secondStage=False \
        Trigger.FPGATrackSim.writeToAOD=True \
        Trigger.FPGATrackSim.writeAdditionalOutputData="$WRITE_UPSTREAM_OUTPUT_DATA" \
        Trigger.FPGATrackSim.outputMonitorFile="monitoring_${TEST_LABEL}.root" \
        Output.AODFileName=${INPUT_AOD_FILE}

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
        ${ATLAS_LOCAL_ROOT}/dcube/current/DCubeClient/python/dcube.py \
            -p -x dcube_last \
            --plotopts=ratio \
            -c ${DCUBE_CONFIG} \
            -M "${PREFIX}" \
            -R "${PREFIX}-previous" \
            -r ${lastref_dir}/IDTPM.${PREFIX}.HIST.root \
            IDTPM.${PREFIX}.HIST.root
fi
