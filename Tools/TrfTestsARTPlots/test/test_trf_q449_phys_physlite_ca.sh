#!/bin/bash
#
# art-description: Reco_tf.py q449 RAWtoALL in MT and AODtoDAOD in MP with CA
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-output: ecube*
# art-output: hist_physlite_latest.root
# art-html: ecube

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA \
  --AMI q449 \
  --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data \
  --outputAODFile myAOD.pool.root \
  --athenaopts "RAWtoALL:--threads=${ATHENA_CORE_NUMBER} --nprocs=0" "AODtoDAOD:--threads=0 --nprocs=${ATHENA_CORE_NUMBER}" \
  --preExec 'flags.Exec.FPE=10' \
  --maxEvents -1

rc1=$?
echo "art-result: ${rc1} Reco_tf_q449_mt" 

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

echo "============ checkxAOD myAOD.pool.root" | tee -a xaod_sizes.log
checkxAOD myAOD.pool.root | tee -a xaod_sizes.log
echo "============ checkxAOD DAOD_PHYS.art.pool.root" | tee -a xaod_sizes.log
checkxAOD DAOD_PHYS.art.pool.root | tee -a xaod_sizes.log
echo "============ checkxAOD DAOD_PHYSLITE.art.pool.root" | tee -a xaod_sizes.log
checkxAOD DAOD_PHYSLITE.art.pool.root | tee -a xaod_sizes.log
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
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449_ca/v7/hist_physlite_2500.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/q449_ca/v4/dcube_config_hist_physlite_2402.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc5=$?
echo "art-result: ${rc5} dcube_physlite" 

# Collect xAOD sizes from logs
echo "============ trf_xaod_sizes_art.py"
mkdir -p ecube/images
datestamp=$AtlasBuildStamp
arch=$CMTCONFIG
trf_xaod_sizes_art.py --date $datestamp --arch $arch --logfile xaod_sizes.log
rc6=$?
echo "art-result: ${rc6} parse_xaod_arttest" 

# Create ratio plots from TH1F histo files
echo "============ trf_ratioplot_art.py"
trf_ratioplot_art.py --reffile ${dcubeRef} --testfile hist_physlite_latest.root
rc7=$?
echo "art-result: ${rc7} ratioplots" 

# Move PNG and webfiles to ecube directory
echo "============ move files to ecube directory"
mv *.png ecube/images
get_files trf_index.php
sed "s/Plot comparison/TrfTestsARTPlots ${datestamp}/g" trf_index.php > index.php
get_files trf_stylesheet.css
mv index.php ecube
mv trf_stylesheet.css ecube
echo "============ done "
