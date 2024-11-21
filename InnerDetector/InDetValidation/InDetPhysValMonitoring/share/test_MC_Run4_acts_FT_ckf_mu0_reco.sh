#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only FastTracking recontruction, acts activated

ArtInFile=$1
dcubeRef=$2
shift 2
idpvmOpts=("$@")

nEvents=1000

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
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
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-ckf-athena" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

# Run with Athena ambi. resolution
run "Reconstruction-ckf" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --preExec 'all:flags.Tracking.doITkFastTracking=True; flags.Tracking.writeExtendedSi_PRDInfo=True; flags.Tracking.doStoreSiSPSeededTracks=True; flags.Tracking.ITkActsValidateTracksPass.storeSiSPSeededTracks=True;' \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${nEvents}

reco_rc=$?

# don't stop right away on an ERROR message ($?=68)
if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --validateExtraTrackCollections "SiSPSeededTracksActsValidateTracksTrackParticles" \
    ${idpvmOpts[@]}

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-ckf-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.root \
    idpvm.root

# Compare performance WRT legacy Athena
run "dcube-ckf-athena" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_athena \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${dcubeRef} \
    -M "acts" \
    -R "athena" \
    idpvm.root
