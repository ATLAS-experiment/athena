#!/bin/bash
set -e


source FPGATrackSim_CommonEnv.sh

TEST_LABEL="F200-InsideOut"

if [ -z $1 ]; then
    xAODOutput="FPGATrackSim_${TEST_LABEL}_AOD.root"
else # this is useful when using the same script for ART
    xAODOutput=$1
fi

run_InsideOut(){
    python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
        --evtMax=${RDO_EVT_ANALYSIS} \
        --filesInput=${RDO_ANALYSIS} \
        Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
        Trigger.FPGATrackSim.bankDir=${BANKS_5L} \
        Trigger.FPGATrackSim.runCKF=$RUN_CKF \
        Trigger.FPGATrackSim.region=0 \
        Trigger.FPGATrackSim.spacePoints=False \
        Trigger.FPGATrackSim.tracking=True \
        Trigger.FPGATrackSim.sampleType=$SAMPLE_TYPE \
        Trigger.FPGATrackSim.Hough.genScan=True \
        Trigger.FPGATrackSim.doEDMConversion=True \
        Trigger.FPGATrackSim.doOverlapRemoval=False \
        Trigger.FPGATrackSim.writeToAOD=True \
        Trigger.FPGATrackSim.outputMonitorFile="monitoring_${TEST_LABEL}.root" \
        Output.AODFileName=$xAODOutput
}

echo "... Running ${TEST_LABEL} analysis"
run_InsideOut
ls -l
echo "... analysis on RDO, this part is done ..."




if [ -z "$ArtJobType" ];then # skip file check for ART (this has already been done in CI)
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
    ls -l
    echo "... Inside-Out on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
