#!/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

TEST_LABEL="F410"
xAODOutput="FPGATrackSim_${TEST_LABEL}.AOD.pool.root"

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
    --skipEvents=$SKIP_EVENTS \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.runCKF=False \
    Trigger.FPGATrackSim.pipeline='F-410' \
    Trigger.FPGATrackSim.GNN.graphTool=graphTool.ModuleMap \
    Trigger.FPGATrackSim.GNN.moduleMapTol=0.5 \
    Trigger.FPGATrackSim.GNN.moduleMapPath=$GNN_MODULE_MAP \
    Trigger.FPGATrackSim.GNN.MLModelPath=$GNN_METRIC_LEARNING \
    Trigger.FPGATrackSim.GNN.GNNModelPath=$GNN_ONNX_MODEL \
    Trigger.FPGATrackSim.GNN.doGNNRootOutput=False \
    Trigger.FPGATrackSim.GNN.doGNNTracking=True \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.mapsDir=$MAPS_9L \
    Trigger.FPGATrackSim.regionList="34,98,162,226,290,354,418,482,546,610,674,738,802,866,930,994,1058,1122,1186,1250" \
    Trigger.FPGATrackSim.oldRegionDefs=False \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.writeClustersToAOD="$WRITE_XAOD_CLUSTERS" \
    Trigger.FPGATrackSim.bankDir=$BANKS_9L \
    Trigger.FPGATrackSim.FakeNNonnxFile1st=$ONNX_INPUT_FAKE_2ND \
    Trigger.FPGATrackSim.ParamNNonnxFile1st=$ONNX_INPUT_PARAM_2ND \
    Trigger.FPGATrackSim.FakeNNonnxFile2nd=$ONNX_INPUT_FAKE_2ND \
    Trigger.FPGATrackSim.ParamNNonnxFile2nd=$ONNX_INPUT_PARAM_2ND \
    Trigger.FPGATrackSim.NNCartesianCoordinates=False \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring_${TEST_LABEL}.root" \
    Trigger.FPGATrackSim.doOverlapRemoval=True \
    Trigger.FPGATrackSim.writeOfflPRDInfo=True \
    Trigger.FPGATrackSim.writeAdditionalOutputData=False
}
run_F410
if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... ${TEST_LABEL} pipeline on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
