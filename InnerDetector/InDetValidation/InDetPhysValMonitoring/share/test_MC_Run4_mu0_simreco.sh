#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only recontruction, acts activated

ArtInFile=$1
dcubeRef=$2
maxEvents=$3

lastref_dir=last_results
dcubeXml=dcube_ART_IDPVMPlots_ITk.xml

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

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
    # Only report hard failures for 21.9 vs master tests since both
    # branches are unlikely to ever match perfectly
    [ "${name}" = "dcube-21p9" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

run "Simulation" \
    Sim_tf.py \
    --CA \
    --conditionsTag "default:${condition}" \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
    --geometryVersion "default:${geometry}" \
    --inputEVNTFile ${ArtInFile} \
    --outputHITSFile HITS.root \
    --maxEvents ${maxEvents} \
    --imf False

run "Digitization"\
    Digi_tf.py \
    --CA \
    --conditionsTag "default:${condition}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --geometryVersion "default:${geometry}" \
    --inputHITSFile HITS.root \
    --jobNumber 568 \
    --maxEvents -1 \
    --outputRDOFile RDO.root \
    --preInclude 'HITtoRDO:Campaigns.PhaseIINoPileUp' \
    --postInclude 'PyJobTransforms.UseFrontier'

run "Reconstruction" \
    Reco_tf.py --CA \
    --inputRDOFile RDO.root \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doTightPrimary \
    --OnlyTrackingPreInclude \
    --doHitLevelPlots

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-21p9" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_21p9 \
    -c ${dcubeXmlAbsPath} \
    -r ${dcubeRef} \
    idpvm.root

run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.root \
    idpvm.root
