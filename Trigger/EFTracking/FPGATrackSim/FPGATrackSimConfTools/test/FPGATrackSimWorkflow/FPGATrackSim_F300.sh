#!/bin/bash
set -e

TEST_LABEL="F300"
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

run_F300(){
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --filesInput=${RDO_ANALYSIS} \
    --evtMax=${RDO_EVT_ANALYSIS} \
    Output.AODFileName=$xAODOutput \
    Trigger.FPGATrackSim.doEDMConversion=True \
    Trigger.FPGATrackSim.runCKF=$RUN_CKF \
    Trigger.FPGATrackSim.mapsDir=${MAPS_9L} \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.pipeline="F-300" \
    Trigger.FPGATrackSim.spacePoints=True \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
    Trigger.FPGATrackSim.bankDir=${BANKS_9L} \
    Trigger.FPGATrackSim.outputMonitorFile="monitoring${TEST_LABEL}.root"
}


echo "... Running ${TEST_LABEL} analysis"
run_F300
ls -l
echo "... analysis on RDO, this part is done ..."

echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
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