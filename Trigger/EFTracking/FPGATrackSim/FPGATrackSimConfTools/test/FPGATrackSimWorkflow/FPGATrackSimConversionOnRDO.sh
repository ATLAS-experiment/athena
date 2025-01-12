#!/bin/bash

set -e


GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_EVT=200
BANKS="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/banks_9L/"
MAPS="maps_9L/OtherFPGAPipelines/v0.22"

echo "... analysis on RDO"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.sampleType='skipTruth' \
    Trigger.FPGATrackSim.bankDir=${BANKS} \
    Trigger.FPGATrackSim.doEDMConversion=True  

ls -l
echo "... analysis on RDO, this part is done ..."
