#!/bin/bash

PREFIX_F100="F100"
PREFIX_F100Sim="F100-Sim"
PREFIX_C100="C100"

SampleName='ttbar_pu200'
nEvents=20
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
    -n ${nEvents} 

run "F100-Simulation" \
  FPGATrackSim_F100_RecoTf.sh \
    -i ${InputRDOfiles} \
    -o "AOD.${PREFIX_F100Sim}.root" \
    -c \
    -n ${nEvents} 

run "F100" \
    F100.sh \
    -i ${InputRDOfiles} \
    -o "AOD.${PREFIX_F100}.root" \
    -c \
    -n ${nEvents}


# Run IDPVM
run "IDPVM-C100" \
    runIDPVM.py \
    --filesInput "AOD.${PREFIX_C100}.root" \
    --outputFile idpvm.${PREFIX_C100}.root \
    --OnlyTrackingPreInclude \
    --doActs

run "IDPVM-F100-Sim" \
    runIDPVM.py \
    --filesInput "AOD.${PREFIX_F100Sim}.root" \
    --outputFile idpvm.${PREFIX_F100Sim}.root \
    --OnlyTrackingPreInclude \
    --doActs

run "IDPVM-F100" \
    runIDPVM.py \
    --filesInput "AOD.${PREFIX_F100}.root" \
    --outputFile idpvm.${PREFIX_F100}.root \
    --OnlyTrackingPreInclude \
    --doActs


# Run dcube
run "dcube-Sim_Vs_Integration" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_sim_vs_integration \
    --plotopts=ratio \
    -c ${dcubeXml} \
    -M ${PREFIX_F100} \
    -R ${PREFIX_F100Sim} \
    -r idpvm.${PREFIX_F100Sim}.root \
    idpvm.${PREFIX_F100}.root

run "dcube-C100_Vs_Integration" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_c100_vs_integration \
    --plotopts=ratio \
    -c ${dcubeXml} \
    -M ${PREFIX_F100} \
    -R ${PREFIX_C100} \
    -r idpvm.${PREFIX_C100}.root \
    idpvm.${PREFIX_F100}.root

run "dcube-C100_Vs_Sim" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_c100_vs_sim \
    --plotopts=ratio \
    -c ${dcubeXml} \
    -M ${PREFIX_F100Sim} \
    -R ${PREFIX_C100} \
    -r idpvm.${PREFIX_C100}.root \
    idpvm.${PREFIX_F100Sim}.root