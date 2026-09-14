#!/bin/bash
set -e

source FPGATrackSim_CommonEnv.sh
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

echo "... Now generating 5L Banks"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput=${RDO_SINGLE_MUON} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.Hough.genScan=True \
    Trigger.FPGATrackSim.Hough.secondStage=False \
    Trigger.FPGATrackSim.GenScan.noCuts=False \
    IOVDb.GlobalTag=${conditions} \
    Trigger.FPGATrackSim.mapsDir=${MAPS_5L}
ls -l
echo "... Banks generation, this part is done ..."
