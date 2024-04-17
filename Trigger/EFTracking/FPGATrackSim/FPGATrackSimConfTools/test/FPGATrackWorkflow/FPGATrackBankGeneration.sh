#!/bin/bash
set -e

GEO_TAG="ATLAS-P2-RUN4-03-00-00"
export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

RDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/${GEO_TAG}/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"
RDO_EVT=200
MAPS="maps_9L/"


echo "... Banks generation"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput=${RDO} \
    --evtMax=${RDO_EVT} \
    Trigger.FPGATrackSim.mapsDir=${MAPS}
ls -l
echo "... Banks generation, this part is done ..."
