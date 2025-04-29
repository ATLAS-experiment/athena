#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

COMBINED_MATRIX="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/banks_9L/combined_matrix.root"
MAPS="maps_9L/OtherFPGAPipelines/v0.22"

echo "... const generation on combined matrix file"
python -m FPGATrackSimBankGen.FPGATrackSimBankConstGenConfig \
    Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx=${COMBINED_MATRIX} \
    Trigger.FPGATrackSim.mapsDir=${MAPS} \
    --evtMax=1
ls -l
echo "... const generation on combined matrix file, this part is done ..."
