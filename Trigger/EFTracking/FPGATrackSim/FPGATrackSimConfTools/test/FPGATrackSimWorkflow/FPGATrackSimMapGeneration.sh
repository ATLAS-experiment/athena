#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
WRAPPER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/Wrappers/v0.10/FPGATrackSimWrapper.root"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

STD_PREFIX="MyMaps_std"
run_map_maker() {
    python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput=${WRAPPER} \
    OutFileName=${STD_PREFIX} \
    Trigger.FPGATrackSim.region=0 \
    GeoModel.AtlasVersion=${GEO_TAG}
}

INSIDEOUT_PREFIX="MyMaps_insideOut"
run_map_maker_for_insideOut() {
    python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput=${WRAPPER} \
    OutFileName=${INSIDEOUT_PREFIX} \
    Trigger.FPGATrackSim.region=33 \
    doInsideOut=True \
    Trigger.FPGATrackSim.spacePoints=False \
    Trigger.FPGATrackSim.oldRegionDefs=False \
    KeyString="plane 0" \
    GeoModel.AtlasVersion=${GEO_TAG}
}

echo "Running map maker"
run_map_maker
echo "Now running map maker for insideOut"
run_map_maker_for_insideOut
echo "Maps Made, this part is done ..."
ls -l

if cmp -s <(find . -name "${STD_PREFIX}*pmap" -exec cat {} +) \
           <(find . -name "${INSIDEOUT_PREFIX}*pmap" -exec cat {} +); then
    echo "pmap should not be the same for the inside-out"
    exit 1
else
    echo "pmap files are different. All look good"
fi
