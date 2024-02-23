#!/bin/sh
#
# art-description: Reco_tf runs on 2011 Heavy Ion data with HardProbes stream (contact Andrzej Olszewski and Iwona Grabowska-Bold)
# art-athena-mt: 4
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=4
Reco_tf.py \
--CA \
--multithreaded \
--inputBSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data11_hi.00193321.physics_HardProbes.merge.RAW._lb0050._SFO-9._0002.1 \
--maxEvents 10 \
--conditionsTag "COMCOND-BLKPA-RUN1-07" \
--geometryVersion "ATLAS-R1-2011-02-00-00" \
--autoConfiguration 'everything' \
--outputAODFile myAOD.pool.root \
--preInclude "all:HIRecConfig.HIModeFlags.HImode" \
--preExec "all:flags.Reco.EnableZDC=False;flags.Reco.EnableTrigger=False" \
--postExec 'all:from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags,"/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1"));cfg.merge(addOverride(flags,"/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v1"));cfg.getCondAlgo("LArADC2MeVCondAlg").CompleteDetector=False' \

RES=$?
echo "art-result: $RES Reco"
