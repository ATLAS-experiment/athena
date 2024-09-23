#!/bin/bash
set -e

RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"
RDO_EVT=200

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH
BANKS="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/banks_9L/"
MAPS="maps_9L/"

echo "... analysis on RDO"

python -m FPGATrackSimConfTools.FPGATrackSimDataPrepConfig \
    --evtMax=${RDO_EVT} \
    --filesInput=${RDO} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    Trigger.FPGATrackSim.bankDir=${BANKS} \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.spacePoints=True \
    Trigger.FPGATrackSim.sampleType='singleMuons' \
    Trigger.FPGATrackSim.doEDMConversion=True  \
    Trigger.FPGATrackSim.writeToAOD=True \
    Trigger.FPGATrackSim.outputMonitorFile="monitoringDataPrep.root" \
    Output.AODFileName="FPGATrackSimCITestAOD.root"

ls -l
echo "... DP pipeline on RDO, this part is done now checking the xAOD"
checkxAOD.py FPGATrackSimCITestAOD.root
