#!/usr/bin/bash
NEVENTS=${1}
export TRF_ECHO=1;
FullCPAlgorithmsTest_CA.py \
        --block-config \
        --data-type data \
        --physlite \
	--evtMax 100000 \
        --perfmon 'fullmonmt' \
        --input-file "/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/cpanalysis/data-ttbar/DAOD_PHYSLITE.myOutput.pool.root" > log.CPAnalysis 2>&1;
ecode=$?
echo ${ecode} > __exitcode;
echo "leaving with code ${ecode}: successful run" >> log.CPAnalysis;
