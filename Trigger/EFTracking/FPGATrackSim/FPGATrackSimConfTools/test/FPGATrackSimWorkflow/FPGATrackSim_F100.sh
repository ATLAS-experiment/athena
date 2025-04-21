#!/bin/bash
set -e

TEST_LABEL="F100"
xAODOutput="FPGATrackSim_${TEST_LABEL}_AOD.root"

FWRD_ARGS=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        -o|--output)
            xAODOutput="$2"
            shift 2
            ;;
        *)
            # Collect all other arguments to forward
            FWRD_ARGS+=("$1")
            shift
            ;;
    esac
done
source FPGATrackSim_CommonEnv.sh "${FWRD_ARGS[@]}"

echo "... analysis on RDO"

python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT_ANALYSIS \
    --filesInput=$RDO_ANALYSIS \
    --skipEvents=${SKIP_EVENTS} \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.pipeline='F-100' \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.oldRegionDefs=True \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.writeAdditionalOutputData="$WRITE_UPSTREAM_OUTPUT_DATA" \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring_${TEST_LABEL}.root"


if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... DP pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
