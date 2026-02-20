#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATADIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO"

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Reco_tf.py \
      --CA 'all:True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --multithreaded 'True' \
      --autoConfiguration 'everything' \
      --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
      --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
      --postInclude 'all:PyJobTransforms.UseFrontier' \
      --preInclude 'all:Campaigns.PhaseIIPileUp200' \
      --steering 'doRAWtoALL' \
      --preExec 'all:flags.Tracking.doITkFastTracking=False' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --inputRDOFile "${DATADIR}/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/*" \
      --outputAODFile 'myAOD.pool.root' \
      --jobNumber '1' >  __log.txt 2>&1;

echo $? > __exitcode;
