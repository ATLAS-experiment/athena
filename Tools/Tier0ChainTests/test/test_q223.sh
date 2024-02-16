#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: main/Athena
# art-include: 23.0/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: 24.0/Athena
# art-athena-mt: 8                                                                                                                                     
# Added "preExec" here, because it was needed to disable dynamic alignment wrt q223 as discussed in ATLASRECTS-5783 (changed InDetGeometryFlags.useDynamicAlignFolders to false wrt q223).
# Updated to data18 input file (q223 uses data15 input file)

# Enabled new monitoring.  
                                                                                           
# The postExec from q223 is not needed anymore as mentioned in ATLASRECTS-6276. Therefore, setting it to empty string here. The original postExec in the AMI tag is: 'postExec': {'all': ['from AthenaCommon.AppMgr import ServiceMgr;import MuonRPC_Cabling.MuonRPC_CablingConfig;ServiceMgr.MuonRPC_CablingSvc.RPCTriggerRoadsfromCool=False']


Reco_tf.py \
--athenaopts='--threads=8' \
--AMI=q223 \
--CA \
--preExec  'pass' \
--postExec 'pass' \
--inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/q223_input_data18/data18_comm.00353055.physics_MinBias.daq.RAW._lb0062._SFO-2._0001.data \
--conditionsTag='CONDBR2-BLKPA-RUN2-11' \
--geometryVersion='ATLAS-R2-2016-01-00-01' \
--maxEvents=500 \
--outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --outputHISTFile=myHIST.root --imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 20 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Diff"

