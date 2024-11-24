#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh
TEST_LABEL="F200"

if [ -z $1 ]; then
    xAODOutput="FPGATrackSim_${TEST_LABEL}_AOD.root"
else # this is useful when using the same script for ART
    xAODOutput=$1
fi

echo "... Running ${TEST_LABEL} analysis"
run_F200(){
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT_ANALYSIS \
    --filesInput=$RDO_ANALYSIS \
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
    Trigger.FPGATrackSim.outputMonitorFile="monitoring${TEST_LABEL}.root"
}
run_F200
if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... ${TEST_LABEL} pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi