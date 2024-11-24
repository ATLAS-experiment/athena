#!/bin/bash
set -e


GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_EVT=200
BANKS="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/banks_9L/"
MAPS="maps_9L/OtherFPGAPipelines/v0.21"

echo "... analysis on RDO"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.sampleType='singleMuons' \
    Trigger.FPGATrackSim.bankDir=${BANKS}
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