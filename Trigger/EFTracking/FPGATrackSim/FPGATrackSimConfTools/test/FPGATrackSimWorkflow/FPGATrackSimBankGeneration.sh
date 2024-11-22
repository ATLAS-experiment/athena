#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh

echo "... 9L Banks generation"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.mapsDir=${MAPS_9L}
ls -l
echo "... 9L Banks generation, this part is done ..."

echo "... Now generating 5L Banks"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.Hough.genScan=True \
    Trigger.FPGATrackSim.mapsDir=${MAPS_5L}
ls -l
echo "... Banks generation, this part is done ..."