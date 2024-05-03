#!/bin/bash
#
# art-description: Reco_tf.py q445, HITtoRDO/RDOtoRDOTrigger/RAWtoALL in MT and AODtoDAOD in MP with CA
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-output: hist_physlite_latest.root
# art-output: generated_csv_files.tar.gz
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA \
  --AMI q445 \
  --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/1000events_singleBS.HITS.pool.root \
  --sharedWriter True \
  --steering 'doRDO_TRIG' 'doTRIGtoALL' \
  --outputAODFile myAOD.pool.root \
  --athenaopts "HITtoRDO:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RDOtoRDOTrigger:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --preExec 'flags.Exec.FPE=10' \
  --runNumber=601229 \
  --DataRunNumber=410000 \
  --maxEvents 1000

rc1=$?
echo "art-result: ${rc1} Reco_tf_q445_phys_physlite_mt_mp" 

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
echo "art-result: ${rcderiv} Derivation_tf_q445_phys_physlite_mp_ca"

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

echo "============ xAODCheckerTRFPlots.py DAOD_PHYS.art.pool.root"
xAODCheckerTRFPlots.py --inputFiles DAOD_PHYS.art.pool.root
rcchecker1=$?
echo "art-result: ${rcchecker1} xAODCheckerTRFPlots PHYS"
echo "============ xAODCheckerTRFPlots.py DAOD_PHYSLITE.art.pool.root"
xAODCheckerTRFPlots.py --inputFiles DAOD_PHYSLITE.art.pool.root
rcchecker2=$?
echo "art-result: ${rcchecker2} xAODCheckerTRFPlots PHYSLITE"

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
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q445_ca/v7/hist_physlite_2500.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q445_ca/v4/dcube_config_hist_physlite_2402.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc5=$?
echo "art-result: ${rc5} dcube_physlite" 

# Run trf_getVariables.py to extract variables from DAOD_PHYSLITE.art.pool.root
echo "============ trf_getVariables.py"
get_files trf_getVariables.py
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
lsetup "root recommended"
trf_getVariables.py --inputFile DAOD_PHYSLITE.art.pool.root
rccsv=$?
tar czf generated_csv_files.tar.gz generated_csv_files/
echo "art-result: ${rccsv} trf_getVariables.py"
