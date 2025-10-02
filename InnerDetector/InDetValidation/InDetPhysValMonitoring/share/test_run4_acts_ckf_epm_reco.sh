#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only recontruction, acts activated, electron events

ArtInFile=$1
lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml
n_events=-1

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi
condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

run () {
    name="${1}"
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect these tests to succeed
    if [[ ("${name}" == "dcube-comparison-athena-acts") && ${rc} -ne 255 ]]; then
        rc=0
    fi
    echo "art-result: $rc ${name}"
    return $rc
}

run "Reconstruction-ckf-electron" \
    Reco_tf.py \
    --preExec "flags.Exec.FPE=-1;" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ckf.root \
    --conditionsTag "default:${condition}" \
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

# legacy athena ITk reconstruction
run "Reconstruction-legacy-athena" \
    Reco_tf.py \
    --preExec "flags.Exec.FPE=-1;" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.athena.ckf.root \
    --conditionsTag "default:${condition}" \
    --maxEvents ${n_events}

reco_rc=$?

# don't stop right away on an ERROR message ($?=68)
if [[ $reco_rc != 0 && $reco_rc != 68 ]]; then
    exit $reco_rc
fi

run "IDPVM-legacy-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.ckf.root \
    --outputFile idpvm.athena.ckf.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --HSFlag All \
    --doExpertPlots

ckf_legacy_rc=$?

if [ $ckf_rc != 0 ]; then
    exit_rc=$ckf_rc
else
    exit_rc=$ckf_legacy_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

echo "download latest legacy athena result..."
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

if [ $ckf_legacy_rc == 0 ]; then
    run "dcube-legacy-athena-ckf-last-electron" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_athena_shifter_last \
        -c ${dcubeXmlAbsPath} \
        -r ${lastref_dir}/idpvm.athena.ckf.root \
        idpvm.athena.ckf.root
fi

if [ $ckf_rc == 0 ] && [ $ckf_legacy_rc == 0 ]; then
    # Compare ACTS performance WRT legacy Athena
    run "dcube-comparison-athena-acts" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_athena_acts_comparison \
        -c ${dcubeXmlTechEffAbsPath} \
        -r idpvm.athena.ckf.root \
        -M "acts" \
        -R "athena" \
        idpvm.ckf.root
fi

exit $exit_rc
