#!/bin/bash
# art-description: Niglty test for an Offline analysis in IDTPM, also comparing against IDPVM
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

input_AOD=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ttbar_mu0_forIDTPM.AOD.root

dcubeXml_IDTPM_OfflTrkAna=dcube_config_IDTPM_OfflTrkAna.xml
IDPVMtoIDTPMcnv=IDPVMtoIDTPMcnv.txt
dcubeXml_IDTPMcmp=dcube_config_IDTPMcmp.xml

# search in $DATAPATH for matching files
dcubeXml_IDTPM_OfflTrkAna_absPath=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $dcubeXml_IDTPM_OfflTrkAna -print -quit 2>/dev/null )
IDPVMtoIDTPMcnv_absPath=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $IDPVMtoIDTPMcnv -print -quit 2>/dev/null )
dcubeXml_IDTPMcmp_absPath=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $dcubeXml_IDTPMcmp -print -quit 2>/dev/null )

# Don't run if dcube config for nightly cmp is not found
if [ -z "$dcubeXml_IDTPM_OfflTrkAna_absPath" ]; then
    echo "art-result: 1 $dcubeXml_IDTPM_OfflTrkAna not found"
    exit 1
fi

run () {
    name="${1}"
    cmd="${@:2}"
    echo "Running ${name}..."
    echo -e "\n---> ${name}" >> "${cwd}/commands.log"
    echo "${cmd}" >> "${cwd}/commands.log"
    time ${cmd}
    rc=$?
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
    runIDTPM.py \
    --inputFileNames ${input_AOD} \
    --outputFilePrefix IDTPM \
    --trkAnaCfgFile InDetTrackPerfMon/offlTrkAnaConfig.json

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

run "dcube-last_skipRC" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    --plotopts=ratio \
    -c ${dcubeXml_IDTPM_OfflTrkAna_absPath} \
    -r ${lastref_dir}/IDTPM.HIST.root \
    IDTPM.HIST.root

# Don't run IDTPM vs IDPVM comparison if conversion config is not found
if [ -z "$IDPVMtoIDTPMcnv_absPath" ]; then
    echo "art-result: 1 $IDPVMtoIDTPMcnv not found"
    exit 1
fi

# convert IDPVM output to IDTPM's format
echo "Converting IDPVM output for comparison..."
IDTPMcnv.py -i idpvm.root -c ${IDPVMtoIDTPMcnv_absPath} -o IDTPMcnv

# Don't run IDTPM vs IDPVM comparison if dcube config is not found
if [ -z "$dcubeXml_IDTPMcmp_absPath" ]; then
    echo "art-result: 1 $dcubeXml_IDTPMcmp not found"
    exit 1
fi

run "dcube-IDTPMvsIDPVM_skipRC" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_cmp \
    --plotopts=ratio \
    -c ${dcubeXml_IDTPMcmp_absPath} \
    -r idpvm.IDTPMcnv.root \
    -R 'ref=IDPVM' -M 'mon=IDTPM' \
    IDTPM.HIST.root
