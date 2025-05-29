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

# Use the old 9L NN for the NN Track Tool, awaiting new training
ONNX_INPUT_FAKE="${BANKS_9L}ClassificationHT_v5.onnx"
ONNX_INPUT_PARAM="${BANKS_9L}ParamEstimationHT_v5.onnx"
ONNX_INPUT_HIT="${BANKS_9L}Ath_Extrap_v51_6_superBig_0_outsideIN.onnx"
ONNX_INPUT_VOL="${BANKS_9L}HT_detector_v6_3.onnx"

echo "... Running ${TEST_LABEL} analysis"
run_F410(){
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=$RDO_EVT_ANALYSIS \
    --filesInput=$RDO_ANALYSIS \
    --skipEvents=$SKIP_EVENTS \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.runCKF=$RUN_CKF \
    Trigger.FPGATrackSim.pipeline='F-410' \
    Trigger.FPGATrackSim.GNN.graphTool=graphTool.MetricLearning \
    Trigger.FPGATrackSim.GNN.moduleMapPath=$GNN_MODULE_MAP \
    Trigger.FPGATrackSim.GNN.MLModelPath=$GNN_METRIC_LEARNING \
    Trigger.FPGATrackSim.GNN.GNNModelPath=$GNN_ONNX_MODEL \
    Trigger.FPGATrackSim.GNN.doGNNRootOutput=True \
    Trigger.FPGATrackSim.GNN.doGNNTracking=True \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L \
    Trigger.FPGATrackSim.region=34 \
    Trigger.FPGATrackSim.oldRegionDefs=False \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.bankDir=$BANKS_9L \
    Trigger.FPGATrackSim.FakeNNonnxFile1st=$ONNX_INPUT_FAKE \
    Trigger.FPGATrackSim.ParamNNonnxFile1st=$ONNX_INPUT_PARAM \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring${TEST_LABEL}.root"
}
run_F410
if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... ${TEST_LABEL} pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi