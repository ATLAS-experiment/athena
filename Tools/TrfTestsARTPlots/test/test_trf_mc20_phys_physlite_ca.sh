#!/bin/bash
#
# art-description: Derivation_tf.py mc20 in MP and CA
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: dcube*
# art-output: hist_physlite_latest.root
# art-html: dcube_physlite

export ATHENA_CORE_NUMBER=8
AODFILE=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/input/v0/1000events_mc20_ttbar.AOD.pool.root 

Derivation_tf.py \
  --CA \
  --inputAODFile $AODFILE \
  --outputDAODFile art.pool.root \
  --formats PHYS PHYSLITE \
  --multiprocess True \
  --sharedWriter True \
  --preExec 'flags.Exec.FPE=10' \
  --maxEvents 1000

rcderiv=$?
echo "art-result: ${rcderiv} Derivation_tf_mc20_phys_physlite_ca" 

# Check for FPEs in the logiles
test_trf_check_fpe.sh
fpeStat=$?

echo "art-result: ${fpeStat} FPEs in logfiles"

echo "============ checkxAOD AOD.27162646._000001.pool.root.1"
checkxAOD $AODFILE
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

echo "============ xAODHistSize DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,tmp.AOD"
xAODHistSize.py --outputHISTFile hist_physlite_latest.root --xAODFiles DAOD_PHYSLITE.art.pool.root,DAOD_PHYS.art.pool.root,$AODFILE
rc4=$?
echo "art-result: ${rc4} xAODHistSize" 

# dcube references
echo "============ dcube references"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/mc20/v7/hist_physlite_2500.root"
dcubeXML="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/mc20/v4/dcube_config_hist_physlite_2402.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId PHYSLITETest -c ${dcubeXML} -r ${dcubeRef} -x dcube_physlite hist_physlite_latest.root
rc5=$?
echo "art-result: ${rc5} dcube_physlite" 
