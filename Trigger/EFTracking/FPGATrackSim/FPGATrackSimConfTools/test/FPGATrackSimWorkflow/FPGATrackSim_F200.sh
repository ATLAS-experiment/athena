#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh


if [ -z $1 ]; then
    xAODOutput="FPGATrackSimCITestAOD.root"
else # this is useful when using the same script for ART
    xAODOutput=$1
fi

echo "... Running F200 analysis"

python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT \
    --filesInput=$RDO \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.runCKF=$RUN_CKF \
    Trigger.FPGATrackSim.pipeline='F-200' \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.bankDir=$BANKS_9L \
    Trigger.FPGATrackSim.outputMonitorFile="monitoringF200.root" \


if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... F200 pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi