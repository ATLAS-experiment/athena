#!/bin/sh -x
#
# art-description: AOD to DAOD_PHYSVAL to NTUP_PHYSVAL and WEB display
# art-type: grid
# art-input: mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8453_s3873_r13829
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input-nfiles: 1
# art-output: DAOD_PHYSVAL.OUT.root
# art-output: NTUP_PHYSVAL.root
# art-output: PHYSVAL_WEB
# art-athena-mt: 4

run () {
  name="${1}"
  cmd="${@:2}"
  echo "Running transform for ${name}\n"
  time ${cmd}
  rc=$?
  echo "art-result: $rc ${name}"
  return $rc
}

checkstep () {
  if [ $? != 0 ]
  then
    exit $?
  else
    echo "${1} Succeeded"
  fi
}


export ATHENA_CORE_NUMBER=4

echo " "
echo " ********** Making DAOD_PHYSVAL **********"
echo " "
run "DAOD_PHYSVAL" Derivation_tf.py \
    --athenaMPMergeTargetSize "DAOD_*:0" \
    --formats "PHYSVAL" \
    --multiprocess "True" \
    --sharedWriter "True" \
    --inputAODFile ${ArtInFile} \
    --outputDAODFile "OUT.root" \
    --maxEvents 10000

checkstep "DAOD_PHYSVAL"


export ATHENA_CORE_NUMBER=1

echo " "
echo " ********** Making NTUP_PHYSVAL **********"
echo " "
run "NTUP_PHYSVAL" Derivation_tf.py \
    --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
    --outputNTUP_PHYSVALFile "NTUP_PHYSVAL.root" \
    --validationFlags doExample, doMET, doEgamma, doInDet, doTau, doJet, doBtag, doMuon, doZee, doTopoCluster, doPFlow_FlowElements \
    --format NTUP_PHYSVAL \
    --maxEvents 10000

checkstep "NTUP_PHYSVAL"


echo " "
echo " ********** Making WEB display **********"
echo " "
mkdir PHYSVAL_WEB
domains="SquirrelPlots, TopoClusters, PFlow, Muons, Electron, Photon, Jets, BTag, Tau, MET"
# Reference is manually updated in:
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/DerivationFrameworkPhysicsValidation/
for slice in ${domains}
do 
    physval_make_web_display.py \
	--ratio \
	--reffile Ref:/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkPhysicsValidation/PHYSVAL_all_reference.root \
	--title Test NTUP_PHYSVAL.root \
	--outdir PHYSVAL_WEB/${slice} \
	--startpath ${slice} \
	--jsRoot	
done

echo  "art-result: $? PHYSVAL web"

echo "Done"
