#!/bin/bash
set -e


GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH
WRP_EVT=200
WRAPPER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/FPGATrackSimWrapper.root"
BANKS="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/banks/"
MAPS="maps/"

echo "... analysis on wrapper"
PY_STATUS=0
stderrFile=$(mktemp)
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=${WRP_EVT} \
    Trigger.FPGATrackSim.wrapperFileName=${WRAPPER} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.bankDir=${BANKS}
ls -l
echo "... analysis on wrapper, this part is done ..."

echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
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