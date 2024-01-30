#!/bin/bash
#
# art-description: Reco_tf.py q443, HITtoRDO/RDOtoRDOTrigger/RAWtoALL in MT and AODtoDAOD in MP with CA
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-output: hist_physlite_latest.root
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA "all:True" "RDOtoRDOTrigger:False" \
  --AMI q443 \
  --steering doRDO_TRIG doTRIGtoALL \
  --outputAODFile myAOD.pool.root \
  --athenaopts "HITtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" \
  --preExec 'HITtoRDO:flags.Exec.FPE=10' 'RAWtoALL:flags.Exec.FPE=10' \
  --postExec 'RDOtoRDOTrigger:from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10;' \
  --runNumber=410470 \
  --DataRunNumber=310000 \
  --maxEvents 2000

rc1=$?
echo "art-result: ${rc1} Reco_tf_q443_phys_physlite_mt_mp" 

# Fail-over if 21.0/RDOtoRDOTrigger container within a container does not work switch it off
if [ "$rc1" -ne "0" ]; then
Reco_tf.py --CA \
  --AMI q443 \
  --steering doRAWtoALL \
  --outputAODFile myAOD.pool.root \
  --athenaopts "HITtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" \
  --preExec 'flags.Exec.FPE=10' \
  --runNumber=410470 \
  --DataRunNumber=310000 \
  --maxEvents 2000
  rcfail=$?
  echo "art-result: ${rcfail} Reco_tf_q443_phys_physlite_mt_mp_fail"
fi

Derivation_tf.py \
  --CA \
  --inputAODFile myAOD.pool.root \
  --outputDAODFile art.pool.root \
  --sharedWriter True \
  --multiprocess True \
  --formats PHYS PHYSLITE \
  --preExec 'flags.Exec.FPE=10' \
  --maxEvents -1

rcderiv=$?
echo "art-result: ${rcderiv} Derivation_tf_q449_phys_physlite_mp_ca" 

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

echo "============ checkxAOD myAOD.pool.root"
checkxAOD myAOD.pool.root
echo "============ checkxAOD DAOD_PHYS.art.pool.root"
checkxAOD DAOD_PHYS.art.pool.root
echo "============ checkxAOD DAOD_PHYSLITE.art.pool.root"
checkxAOD DAOD_PHYSLITE.art.pool.root
rc2=$?
echo "art-result: ${rc2} checkxAOD" 

echo "============ xAODHist DAOD_PHYSLITE.art.pool.root"
xAODHist.py --analysis --outputHISTFile hist_physlite_latest.root DAOD_PHYSLITE.art.pool.root 
rc3=$?
echo "art-result: ${rc3} xAODHist DAOD_PHYSLITE.art.pool.root" 

echo "============ xAODHistSize DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,myAOD.pool.root"
xAODHistSize.py --outputHISTFile hist_physlite_latest.root --xAODFiles DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,myAOD.pool.root
rc4=$?
echo "art-result: ${rc4} xAODHistSize" 

# dcube references
echo "============ dcube references"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q443_ca/v6/hist_physlite_24017.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q443_ca/v4/dcube_config_hist_physlite_2402.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc5=$?
echo "art-result: ${rc5} dcube_physlite" 
