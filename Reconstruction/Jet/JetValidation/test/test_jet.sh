#!/bin/bash
#
# art-description: RAWtoALL (q443 test), PHYSVAL production and NTUP_PHYSVAL for jets
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: *.root
# art-output: dcube
# art-html: dcube

export ATHENA_CORE_NUMBER=8
Reco_tf.py \
  --AMI q443 \
  --CA "all:True" "RDOtoRDOTrigger:False" \
  --sharedWriter True \
  --steering 'doRDO_TRIG' 'doTRIGtoALL' \
  --outputAODFile myAOD.pool.root \
  --athenaopts "HITtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10;' \
  --maxEvents 1000

rc1=$?
echo "art-result: ${rc1} Reco_tf"

Derivation_tf.py \
  --CA \
  --inputAODFile myAOD.pool.root \
  --outputDAODFile pool.root \
  --sharedWriter True \
  --multiprocess True \
  --formats PHYSVAL \
  --postExec 'from AthenaAuditors.AthenaAuditorsConf import FPEAuditor;FPEAuditor.NStacktracesOnFPE=10' \
  --maxEvents -1

rc2=$?
echo "art-result: ${rc2} PHYSVAL"

Derivation_tf.py \
  --CA \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.pool.root" \
  --outputNTUP_PHYSVALFile "nightly_jet.PHYSVAL.root" \
  --validationFlags doJet \
  --format NTUP_PHYSVAL \
  --maxEvents -1

rc3=$?
echo "art-result: ${rc3} NTUP_PHYSVAL"

if [ ${rc3} -eq 0 ]
then
  art.py download --user=artprod --dst=last_results JetValidation test_jet
  # Histogram comparison with DCube
  dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/JetValidation/DCUBE/jet.xml"
  dcubeRef="last_results/nightly_jet.PHYSVAL.root"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p -c ${dcubeXML} -r ${dcubeRef} -x dcube nightly_jet.PHYSVAL.root
  rc4=$?
fi
echo "art-result: ${rc4} dcube"
