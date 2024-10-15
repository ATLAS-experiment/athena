#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_EVT=200
#MAPS="maps_9L/"
MAPS="maps_9L/OtherFPGAPipelines/v0.20"

echo "... Banks generation"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.mapsDir=${MAPS}
ls -l
echo "... Banks generation, this part is done ..."
