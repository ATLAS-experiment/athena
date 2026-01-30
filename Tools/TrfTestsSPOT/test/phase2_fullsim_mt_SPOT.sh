#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATAFILE='/eos/atlas/atlascerngroupdisk/data-art/grid-input/PhaseIIUpgrade/EVNT/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481/'

# Run the job
export TRF_ECHO=1;
ATHENA_CORE_NUMBER=${NTHREADS} \
Sim_tf.py \
      --CA 'True' \
      --multithreaded 'True' \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --conditionsTag 'default:OFLCOND-MC21-SDR-RUN4-02' \
      --geometryVersion 'default:ATLAS-P2-RUN4-03-00-00' \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --simulator 'FullG4MT' \
      --inputEVNTFile ${DATAFILE} \
      --outputHITSFile 'myHITS.pool.root' > __log.txt 2>&1;

echo $? > __exitcode;
