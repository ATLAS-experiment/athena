#!/bin/bash

PREFIX_F100="F100"
PREFIX_C100="C100"

SampleName='ttbar_pu200'
nEvents=1000
dcubeXml="/eos/atlas/atlascerngroupdisk/data-art/grid-input/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/dcube/config/IDPVM-clusterValidation.xml"

run () {
    name="${1}"
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-trk" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

InputRDOfiles=$( getEFTrackSample.py -s ${SampleName} )
if [ ! -f "${InputRDOfiles}" ]; then
    echo "art-result: 1 Sample ${SampleName} not found"
    exit 1
fi


run "C100" \
  runReco_C100_FS.sh \
    -i ${InputRDOfiles} \
    -o "AOD.${PREFIX_C100}.root" \
    -c \
    -n ${nEvents} > ${PREFIX_C100}.txt 2>&1 &

run "F100" \
    F100.sh \
    -i ${InputRDOfiles} \
    -o "AOD.${PREFIX_F100}.root" \
    -c \
    -n ${nEvents} >${PREFIX_F100}.txt 2>&1 &


wait
# Run IDPVM
run "IDPVM-C100" \
    runIDPVM.py \
    --filesInput "AOD.${PREFIX_C100}.root" \
    --outputFile idpvm.${PREFIX_C100}.root \
    --OnlyTrackingPreInclude

run "IDPVM-F100" \
    runIDPVM.py \
    --filesInput "AOD.${PREFIX_F100}.root" \
    --outputFile idpvm.${PREFIX_F100}.root \
    --OnlyTrackingPreInclude


# Run dcube
run "dcube-C100_Vs_Integration" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_c100_vs_integration \
    --plotopts=ratio \
    -c ${dcubeXmlAbsPath} \
    -M ${PREFIX_F100} \
    -R ${PREFIX_C100} \
    -r ${dcubeXml}/idpvm.${PREFIX_C100}.root \
    idpvm.${PREFIX_F100}.root