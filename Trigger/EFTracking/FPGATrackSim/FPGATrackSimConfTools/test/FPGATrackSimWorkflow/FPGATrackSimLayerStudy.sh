#!/bin/bash
set -e

TEST_LABEL="LayerStudy"

FWRD_ARGS=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        *)
            # Collect all other arguments to forward
            FWRD_ARGS+=("$1")
            shift
            ;;
    esac
done
source FPGATrackSim_CommonEnv.sh "${FWRD_ARGS[@]}"

run_LayerStudyFirst(){
    python -m FPGATrackSimConfTools.FPGATrackSimLayerStudyConfig \
        --evtMax=${RDO_EVT_ANALYSIS} \
        --skipEvents=${SKIP_EVENTS} \
        --filesInput=${RDO_ANALYSIS} \
        Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
        Trigger.FPGATrackSim.bankDir=${BANKS_5L} \
        Trigger.FPGATrackSim.region="34" \
        Trigger.FPGATrackSim.sampleType=singleMuons \
        Trigger.FPGATrackSim.Hough.genScan=True \
        Trigger.FPGATrackSim.Hough.threshold=[4] \
        Trigger.FPGATrackSim.oldRegionDefs=False \
        Trigger.FPGATrackSim.layerStudyStage=1 \
        Trigger.FPGATrackSim.GenScan.initialLayerStudy=True \
        Trigger.FPGATrackSim.GenScan.rin=30 \
        Trigger.FPGATrackSim.GenScan.rout=300 \
        Trigger.FPGATrackSim.GenScan.parMin="[-1000, -1000, 0.0, 0.0, -10]" \
        Trigger.FPGATrackSim.GenScan.parMax="[1000, 1000, 1.0, 1.0, 10]"
}

echo "... Running ${TEST_LABEL} analysis"
run_LayerStudyFirst
ls -l
echo "... analysis on RDO, this part is done ..."

if [ -z "$ArtJobType" ];then # skip file check for ART (this has already been done in CI)
    echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd();
    TH1* h = (TH1*)gDirectory->Get("truthxm");
    if ( h == nullptr )
        throw std::runtime_error("oh dear, after all of this there is no truth x_m histogram");
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are zero truth tracks");
    }
}
EOF

    root -b -q genscan.root checkHist.C
    echo "... analysis output verification, this part is done ..."
    ls -l
fi
