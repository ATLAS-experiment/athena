#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only recontruction, acts activated

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

dcuberef_sim=$1
dcuberef_rdo=$2
dcuberef_rec=$3
maxEvents=$4

# Following specify DCube output directories. Set empty to disable.
dcube_sim_fixref="dcube_sim"
dcube_sim_lastref="dcube_sim_last"
dcube_rdo_fixref="dcube_rdo"
dcube_rdo_lastref="dcube_rdo_last"
dcube_rec_fixref="dcube"
dcube_rec_lastref="dcube_last"

hits=physval.HITS.root
rdo=physval.RDO.root
aod=physval.AOD.root
dcubemon_sim=SiHitValid.root
dcubemon_rec=physval.ntuple.root
dcubemon_rdo=RDOAnalysis.root

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubecfg_sim=$artdata/InDetPhysValMonitoring/dcube/config/run4_SiHitValid.xml
dcubecfg_rdo=$artdata/InDetPhysValMonitoring/dcube/config/run4_RDOAnalysis.xml
art_dcube=$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py

lastref_dir=last_results
dcubeXml=dcube_ART_IDPVMPlots_ITk.xml

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# search in $DATAPATH for matching file
dcubeshiftercfg_rec=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeshiftercfg_rec" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

run Sim_tf.py \
    --CA \
    --conditionsTag "default:${condition}" \
    --simulator 'FullG4MT' \
    --postInclude 'default:PyJobTransforms.UseFrontier' 'HitAnalysis.PostIncludes.ITkHitAnalysis'\
    --preInclude 'EVNTtoHITS:Campaigns.PhaseIISimulation' \
    --geometryVersion "default:${geometry}" \
    --inputEVNTFile ${ArtInFile} \
    --outputHITSFile $hits \
    --maxEvents ${maxEvents} \
    --imf False
sim_tf_exit_code=$?
echo "art-result: $sim_tf_exit_code sim"

if [ $sim_tf_exit_code -eq 0 ]  ;then
 echo "download latest result"
 run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
 run ls -la "$lastref_dir"

 # DCube Sim hit plots
 # To be enabled when references are available
 #$art_dcube \
 #    -p -x ${dcube_sim_fixref} \
 #    -c ${dcubecfg_sim} \
 #    -r ${dcuberef_sim} \
 #    ${dcubemon_sim}
 #echo "art-result: $? dcube_sim"

 $art_dcube \
    -p -x ${dcube_sim_lastref} \
    -c ${dcubecfg_sim} \
    -r ${lastref_dir}/${dcubemon_sim} \
    ${dcubemon_sim}
 echo "art-result: $? dcube_sim_last"


 run Digi_tf.py \
    --CA \
    --conditionsTag "default:${condition}" \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --geometryVersion "default:${geometry}" \
    --inputHITSFile $hits \
    --jobNumber 568 \
    --maxEvents -1 \
    --outputRDOFile $rdo \
    --preInclude 'HITtoRDO:Campaigns.PhaseIINoPileUp' \
    --postInclude 'PyJobTransforms.UseFrontier'
 echo "art-result: $? digi"

 run RunRDOAnalysis.py \
    -i $rdo \
    ITkPixel ITkStrip
 echo "art-result: $? RDOAnalysis"

 # To be enabled when references are available
 #echo "compare with a fixed reference for RDOAnalysis"
 #$art_dcube \
 #    -p -x ${dcube_rdo_fixref} \
 #    -c ${dcubecfg_rdo} \
 #    -r ${dcuberef_rdo} \
 #    ${dcubemon_rdo}
 #echo "art-result: $? dcube_rdo"

 echo "compare with last build"
 $art_dcube \
    -p -x ${dcube_rdo_lastref} \
    -c ${dcubecfg_rdo} \
    -r ${lastref_dir}/${dcubemon_rdo} \
    ${dcubemon_rdo}
 echo "art-result: $? dcube_rdo_last"

 run Reco_tf.py --CA \
    --inputRDOFile $rdo \
    --outputAODFile $aod \
    --steering doRAWtoALL
 rec_tf_exit_code=$?
 echo "art-result: $rec_tf_exit_code reco"

 runIDPVM.py \
    --filesInput $aod \
    --outputFile idpvm.root \
    --doTightPrimary \
    --OnlyTrackingPreInclude \
    --doHitLevelPlots
 idpvm_tf_exit_code=$?
 echo "art-result: $idpvm_tf_exit_code idpvm"

 if [ $rec_tf_exit_code -eq 0 ]  ;then

   # To be enabled when references are available
   #echo "compare with a fixed reference"
   #$art_dcube \
   #    -p -x ${dcube_rec_fixref} \
   #    -c ${dcubeshiftercfg_rec} \
   #    -r ${dcuberef_rec} \
   #    ${dcubemon_rec}
   #echo "art-result: $? dcube_rec"

   echo "compare with last build"
   $art_dcube \
     -p -x ${dcube_rec_lastref} \
     -c ${dcubeshiftercfg_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}
   echo "art-result: $? dcube_rec_last"
 fi

fi
