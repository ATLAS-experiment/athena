#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"

# instructions on how to change version of files can be found in https://twiki.cern.ch/twiki/bin/view/Atlas/EFTrackingSoftware
MAP_VERSION="v0.10"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH
MAPS="maps_9L/InsideOut/${MAP_VERSION}/"


if [ -z $1 ]; then
    xAODOutput="FPGATrackSimInsideOutAOD.root"
    RDO_EVT=200
else
    xAODOutput=$1
    RDO_EVT=-1
fi

echo "... analysis on RDO"

python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    --evtMax=${RDO_EVT} \
    --filesInput=${RDO} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    Trigger.FPGATrackSim.region=0 \
    Trigger.FPGATrackSim.spacePoints=False \
    Trigger.FPGATrackSim.tracking=False \
    Trigger.FPGATrackSim.sampleType='singleMuons' \
    Trigger.FPGATrackSim.Hough.genScan=True \
    Trigger.FPGATrackSim.doEDMConversion=False \
    Trigger.FPGATrackSim.writeToAOD=False \
    Trigger.FPGATrackSim.outputMonitorFile="insideOut.root" \
    Output.AODFileName=$xAODOutput


if [ -z $ArtJobType ];then # skip file check for ART (this has already been done in CI)
    ls -l
    echo "... Inside-Out on RDO, this part is done now checking the xAOD"
    checkxAOD.py $xAODOutput
fi
