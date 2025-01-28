#!/bin/bash
set -e

LABEL="F100"

if [ -z $1 ]; then
    xAODOutput="FPGATrackSim_${LABEL}_AOD.root"
else
    xAODOutput=$1
    shift
fi

source FPGATrackSim_CommonEnv.sh "$@"

echo "... analysis on RDO"

python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT_ANALYSIS \
    --filesInput=$RDO_ANALYSIS \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.pipeline='F-100' \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring_${LABEL}.root"


if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... DP pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
