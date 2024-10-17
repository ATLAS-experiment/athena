#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
WRAPPER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/Wrappers/v0.10/FPGATrackSimWrapper.root"

echo "... Maps Making"
python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput=${WRAPPER} \
    OutFileName="MyMaps_" \
    Trigger.FPGATrackSim.region=0 \
    GeoModel.AtlasVersion=${GEO_TAG}
ls -l
echo "... Maps Making, this part is done ..."
