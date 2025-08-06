#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

source FPGATrackSim_CommonEnv.sh
INSIDEOUT_PREFIX="MyMaps_insideOut"
run_5L_map_maker() {
python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput=${RDO_ANALYSIS} \
    OutFileName=${INSIDEOUT_PREFIX} \
    Trigger.FPGATrackSim.region=34 \
    doInsideOut=True \
    Trigger.FPGATrackSim.spacePoints=False \
    KeyString="plane 0" \
    GeoModel.AtlasVersion=${GEO_TAG} \
    --evtMax=200
}

echo "Running map maker for insideOut"
run_5L_map_maker
echo "Maps Made, this part is done ..."
ls -l

