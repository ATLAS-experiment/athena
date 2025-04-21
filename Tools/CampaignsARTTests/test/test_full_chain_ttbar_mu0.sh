#!/bin/bash
# art-description: Run-4 Sim to DAOD_PHYSVAL and output plots via dcube, on non-all-had ttbar with no pile-up
# art-input: mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_wrt_last_nightly
# art-athena-mt: 8

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

file=test_full_chain_mu0.sh
script="`basename \"$0\"`"
number_of_events=4000

#References for producing the output dcube pages
lastref_dir=last_results
# TODO: This one needs to be moved to CVMFS once it's cleaned up by the different domains (~5000 histograms right now)
dcube_xml="${Athena_DIR}/src/Tools/CampaignsARTTests/config/dcube_config_all_domains_reduced.xml"

echo "Executing script ${file}"
echo " "
"$file" ${number_of_events}

echo "Download results from previous nightly"
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "DCUBE_LAST" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
  -p -x dcube_wrt_last_nightly \
  -c ${dcube_xml} \
  -r ${lastref_dir}/NTUP_MERGE_PHYSVAL.root \
  NTUP_MERGE_PHYSVAL.root

checkstep "DCUBECREATION"
exit "Everything done $?"
