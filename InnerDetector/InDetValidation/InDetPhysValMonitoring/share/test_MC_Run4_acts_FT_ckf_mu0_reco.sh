#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only FastTracking recontruction, acts activated

ArtInFile=$1
nEvents=$2
shift 2
idpvmOpts=("$@")

lastref_dir=last_results
dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml

# search in $DATAPATH for matching file
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlTechEffAbsPath" ]; then
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
    echo "art-result: $rc ${name}"
    return $rc
}

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run with ACTS
run "Reconstruction-acts" \
    Reco_tf.py \
    --conditionsTag "default:${conditionsTag}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingRecoPreInclude" \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.ITkActsPass.storeSiSPSeededTracks=True;" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.acts.root \
    --maxEvents ${nEvents}

reco_rc=$?
# don't stop right away on an ERROR message ($?=68)
if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --filesInput AOD.acts.root \
    --outputFile idpvm.acts.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --validateExtraTrackCollections "SiSPSeededTracksActs" \
    ${idpvmOpts[@]}

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-acts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root
