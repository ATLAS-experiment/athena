#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-athena-acts" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

# Run with Athena
run "Reconstruction-athena" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Tracking.doITkFastTracking=True; \
    	       flags.Tracking.writeExtendedSi_PRDInfo=True; \
	       flags.Tracking.doStoreSiSPSeededTracks=True; \
	       flags.Tracking.ITkFastPass.storeSiSPSeededTracks=True;" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.athena.root \
    --maxEvents ${nEvents}    

reco_rc=$?
# don't stop right away on an ERROR message ($?=68)
if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --validateExtraTrackCollections "SiSPSeededTracks" \
    ${idpvmOpts[@]}

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run with ACTS
ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

run "Reconstruction-acts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.ITkActsPass.storeSiSPSeededTracks=True;" \
    --ignorePatterns ${ignore_pattern} \
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

run "dcube-athena-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_shifter_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root

run "dcube-acts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

# Compare performance WRT legacy Athena
run "dcube-athena-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_acts \
    -c ${dcubeXmlTechEffAbsPath} \
    -R "athena" \
    -r idpvm.athena.root \
    -M "acts" \
    idpvm.acts.root
