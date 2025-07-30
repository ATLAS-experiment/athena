#!/bin/bash
#
# art-description: Reco_tf.py q454, HITtoRDO/RDOtoRDOTrigger/RAWtoALL in MT and AODtoDAOD in MP with CA
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-output: hist_physlite_latest.root
# art-output: generated_csv_files.tar.gz
# art-output: physlite_disksize.tar.gz
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
Reco_tf.py \
  --AMI q454 \
  --outputAODFile myAOD.pool.root \
  --preExec 'flags.Exec.FPE=10' \
  --conditionsTag "${conditions}" \
  --runNumber=601229 \
  --DataRunNumber=450000 \
  --maxEvents 1000

rc1=$?
echo "art-result: ${rc1} Reco_tf_q454_phys_physlite_mt_mp" 

stdbuf -i0 -o0 -e0 Derivation_tf.py \
  --inputAODFile myAOD.pool.root \
  --outputDAODFile art.pool.root \
  --sharedWriter True \
  --multiprocess True \
  --formats PHYS PHYSLITE \
  --preExec 'flags.Exec.FPE=10' \
  --maxEvents -1

rcderiv=$?
echo "art-result: ${rcderiv} Derivation_tf_q454_phys_physlite_mp_ca"

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
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q454/v13/hist_physlite_25033.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q454/v0/dcube_config_hist_physlite_24024.xml"
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
LCGVER=`root-config --prefix | grep -o 'LCG_[0-9]*[a-z]*'`
source /cvmfs/sft.cern.ch/lcg/releases/$LCGVER/uproot/*/$LCG_PLATFORM/uproot-env.sh
trf_getVariables.py --inputFile DAOD_PHYSLITE.art.pool.root
rccsv=$?
tar czf generated_csv_files.tar.gz generated_csv_files/
echo "art-result: ${rccsv} trf_getVariables.py"
get_files trf_disksize_get_metrics.py
python trf_disksize_get_metrics.py --inputFile DAOD_PHYSLITE.art.pool.root --isuproot=True
tar czf physlite_disksize.tar.gz physlite_disksize/
echo "art-result: physlite_disksize.tar.gz"
