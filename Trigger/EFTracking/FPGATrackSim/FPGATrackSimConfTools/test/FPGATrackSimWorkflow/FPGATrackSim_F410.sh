#!/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

TEST_LABEL="F410"
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

echo "... Running ${TEST_LABEL} analysis"
run_F410(){
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT_ANALYSIS \
    --filesInput=$RDO_ANALYSIS \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.runCKF=$RUN_CKF \
    Trigger.FPGATrackSim.pipeline='F-410' \
    Trigger.FPGATrackSim.GNN.moduleMapPath=$GNN_MODULE_MAP \
    Trigger.FPGATrackSim.GNN.GNNModelPath=$GNN_ONNX_MODEL \
    Trigger.FPGATrackSim.GNN.doGNNRootOutput=True \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L_GNN \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.bankDir=$BANKS_9L \
    Trigger.FPGATrackSim.FakeNNonnxFile=$ONNX_INPUT_FAKE \
    Trigger.FPGATrackSim.ParamNNonnxFile=$ONNX_INPUT_PARAM \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring${TEST_LABEL}.root"
}
run_F410
if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... ${TEST_LABEL} pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi