#!/bin/bash
# art-description: Niglty test for an Offline analysis in IDTPM, also comparing against IDPVM
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: *.json
# art-output: dcube*
# art-html: dcube_last

input_AOD=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ttbar_mu0_forIDTPM.AOD.root

run () {
    name="${1}"
    cmd="${@:2}"
    echo "Running ${name}..."
    echo -e "\n---> ${name}" >> "${cwd}/commands.log"
    echo "${cmd}" >> "${cwd}/commands.log"
    time ${cmd}
    rc=$?
    ## if _diffOK is in name, then we expect dcube differences, so don't flag as an error
    if [[ $rc == 1 && "${name}" =~ "_diffOK" ]]; then
      rc=0
    fi
    echo "art-result: $rc ${name}"
    ## if _skipRC is in name skip exit condition
    if [[ "${name}" =~ "_skipRC" ]]; then
      return 0
    fi
    if [ $rc != 0 ]; then
        exit $rc
    fi
    return $rc
}

run "IDTPM" \
    runIDTPM_Offl.py \
      --inputFileNames ${input_AOD} \
      --outputFilePrefix IDTPM \
      --doTightPrimary

run "IDPVM" \
    runIDPVM.py \
      --filesInput ${input_AOD} \
      --outputFile idpvm.root \
      --doTightPrimary

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
lastref_dir=last_results
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

## making Dcube XML config on-the-fly for test against last
makeDcubeConfig.py -i IDTPM.HIST.root -c config_dcube_last.xml

run "dcube-last_skipRC" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    --plotopts=ratio \
    -c config_dcube_last.xml \
    -r ${lastref_dir}/IDTPM.HIST.root \
    IDTPM.HIST.root

# convert IDPVM output to IDTPM's format
echo "Converting IDPVM output for comparison..."
IDPVMtoIDTPMConverter.py -i idpvm.root -o idpvm.IDTPMcnv.root --doTightPrimary

## making Dcube XML config on-the-fly for comparison IDPVM-vs-IDTPM
makeDcubeConfig.py -i idpvm.IDTPMcnv.root -c config_dcube_cmp.xml

run "dcube-IDTPMvsIDPVM_diffOK" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_cmp \
    --plotopts=ratio \
    -c config_dcube_cmp.xml \
    -r idpvm.IDTPMcnv.root \
    -R 'ref=IDPVM' -M 'mon=IDTPM' \
    IDTPM.HIST.root
