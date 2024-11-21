#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only recontruction, acts activated, electron events

ArtInFile=$1
lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
n_events=-1

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

run () {
    name="${1}"
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect these tests to succeed
    if [[ ("${name}" == "dcube-ckf-ambi" || "${name}" == "dcube-ckf-athena" || "${name}" == "dcube-ambi-greedy-scored") && ${rc} -ne 255 ]]; then
        rc=0
    fi
    echo "art-result: $rc ${name}"
    return $rc
}

ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:3.+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.+ERROR.+Step.+size.+adjustment.+exceeds.+maximum.+trials,Acts.+FindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+SurfaceError:1,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

run "Reconstruction-ckf-electron" \
    Reco_tf.py \
    --preExec "flags.Exec.FPE=-1;" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsAloneWorkflowFlags" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${n_events}

reco_rc=$?

# don't stop right away on an ERROR message ($?=68)
if [[ $reco_rc != 0 && $reco_rc != 68 ]]; then
    exit $reco_rc
fi


run "IDPVM-ckf-electron" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.ckf.root \
    --OnlyTrackingPreInclude \
    --doTightPrimary \
    --doHitLevelPlots \
    --HSFlag All \
    --doExpertPlots

ckf_rc=$?

if [ $ckf_rc != 0 ]; then
    exit $ckf_rc
fi


echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

if [ $ckf_rc == 0 ]; then
    run "dcube-ckf-last-electron" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_acts_shifter_last \
        -c ${dcubeXmlAbsPath} \
        -r ${lastref_dir}/idpvm.ckf.root \
        idpvm.ckf.root
fi
    
exit $exit_rc
