#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
WRAPPER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/Wrappers/v0.11/FPGATrackSimWrapper.root"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

INSIDEOUT_PREFIX="MyMaps_insideOut"
run_5L_map_maker() {
    python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput="wrapper.root" \
    OutFileName=${INSIDEOUT_PREFIX} \
    Trigger.FPGATrackSim.region=34 \
    doInsideOut=True \
    Trigger.FPGATrackSim.spacePoints=False \
    KeyString="plane 0" \
    GeoModel.AtlasVersion=${GEO_TAG}
}

echo "Running map maker for insideOut"
run_5L_map_maker
echo "Maps Made, this part is done ..."
ls -l

