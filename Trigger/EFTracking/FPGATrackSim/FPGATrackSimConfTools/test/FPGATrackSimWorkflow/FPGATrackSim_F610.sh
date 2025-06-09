#!/bin/bash
set -e

TEST_LABEL="F610"
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

run_F610(){
    python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
        --evtMax=${RDO_EVT_ANALYSIS}\
        --filesInput=${RDO_ANALYSIS} \
        Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
        Trigger.FPGATrackSim.bankDir=${BANKS_5L} \
        Trigger.FPGATrackSim.runCKF=$RUN_CKF \
        Trigger.FPGATrackSim.regionList="34,98,162,226,290,354,418,482,546,610,674,738,802,866,930,994,1058,1122,1186,1250" \
        Trigger.FPGATrackSim.pipeline='F-610' \
        Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
        Trigger.FPGATrackSim.doEDMConversion=True \
        Trigger.FPGATrackSim.Hough.secondStage=True \
        Trigger.FPGATrackSim.writeToAOD=True \
        Trigger.FPGATrackSim.writeClustersToAOD="$WRITE_XAOD_CLUSTERS" \
        Trigger.FPGATrackSim.writeAdditionalOutputData="$WRITE_UPSTREAM_OUTPUT_DATA" \
        Trigger.FPGATrackSim.FakeNNonnxFile1st=$ONNX_INPUT_FAKE \
        Trigger.FPGATrackSim.ParamNNonnxFile1st=$ONNX_INPUT_PARAM \
        Trigger.FPGATrackSim.FakeNNonnxFile2nd=$ONNX_INPUT_FAKE_2ND \
        Trigger.FPGATrackSim.ParamNNonnxFile2nd=$ONNX_INPUT_PARAM_2ND \
        Trigger.FPGATrackSim.ExtensionNNVolonnxFile=$ONNX_INPUT_VOL \
        Trigger.FPGATrackSim.ExtensionNNHitonnxFile=$ONNX_INPUT_HIT \
        Trigger.FPGATrackSim.outputMonitorFile="monitoring_${TEST_LABEL}.root" \
        Trigger.FPGATrackSim.writeOfflPRDInfo=True \
        Output.AODFileName=$xAODOutput 
}

echo "... Running ${TEST_LABEL} analysis"
run_F610
ls -l
echo "... analysis on RDO, this part is done ..."




if [ -z "$ArtJobType" ];then # skip file check for ART (this has already been done in CI)
    echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg_reg34");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st");
    if ( h == nullptr )
        throw std::runtime_error("oh dear, after all of this there is no roads histogram");
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are zero roads");
    }
}
EOF

    root -b -q monitoring.root checkHist.C
    echo "... analysis output verification, this part is done ..."
    ls -l
    echo "... F-610 on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
