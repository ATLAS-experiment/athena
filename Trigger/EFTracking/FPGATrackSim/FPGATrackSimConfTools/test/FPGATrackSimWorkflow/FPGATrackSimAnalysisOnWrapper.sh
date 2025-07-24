#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh

echo "... analysis on wrapper"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.wrapperFileName="wrapper.root" \
    Trigger.FPGATrackSim.regionList="34,98" \
    Trigger.FPGATrackSim.pipeline='F-600' \
    Trigger.FPGATrackSim.doOverlapRemoval=True \
    Trigger.FPGATrackSim.Hough.secondStage=True \
    Trigger.FPGATrackSim.mapsDir=${MAPS_5L} \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.bankDir=${BANKS_5L} 
ls -l
echo "... analysis on wrapper, this part is done ..."

echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg_reg34");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st");
    if ( h == nullptr )
        throw std::runtime_error("oh dear, after all of this there are no roads histogram");
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are zero roads");
    }
}
EOF

root -b -q monitoring.root checkHist.C
echo "... analysis output verification, this part is done ..."
