#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART jobs with MC Sim+Reco mu=0 config

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

dcuberef_sim=$1
dcuberef_rdo=$2
dcuberef_rec=$3

# Following specify DCube output directories. Set empty to disable.
dcube_sim_fixref="dcube_sim"
dcube_sim_lastref="dcube_sim_last"
dcube_rdo_fixref="dcube_rdo"
dcube_rdo_lastref="dcube_rdo_last"
dcube_rec_fixref="dcube_shifter"
dcube_rec_expert_fixref="dcube_expert"
dcube_rec_lastref="dcube_shifter_last"
dcube_rec_expert_lastref="dcube_expert_last"

hits=physval.HITS.root
rdo=physval.RDO.root
aod=physval.AOD.root
dcubemon_sim=HitValid.root
dcubemon_rec=physval.ntuple.root
dcubemon_rdo=RDOAnalysis.root

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubecfg_sim=$artdata/InDetPhysValMonitoring/dcube/config/run2_SiHitValid.xml
dcubecfg_rdo=$artdata/InDetPhysValMonitoring/dcube/config/run2_RDOAnalysis.xml
dcubeshiftercfg_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_baseline.xml
dcubeexpertcfg_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_expert.xml
art_dcube=$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py

lastref_dir=last_results

geotag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

# MC23a simulation config, based on s4162
 run Sim_tf.py \
    --inputEVNTFile   ${ArtInFile} \
    --outputHITSFile  "$hits" \
    --skipEvents      0 \
    --maxEvents       10000 \
    --randomSeed      24304 \
    --simulator       FullG4MT_QS \
    --conditionsTag   default:$conditionsTag \
    --geometryVersion default:$geotag \
    --preInclude      'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --postInclude     'PyJobTransforms.TransformUtils.UseFrontier' 'HitAnalysis.PostIncludes.IDHitAnalysis'
sim_tf_exit_code=$?
echo "art-result: $sim_tf_exit_code sim"

if [ $sim_tf_exit_code -eq 0 ]  ;then

 hadd ${dcubemon_sim} SiHitValid.root TRTHitValid.root

 echo "download latest result"
 run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
 run ls -la "$lastref_dir"
 # DCube Sim hit plots
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_fixref} \
   -c ${dcubecfg_sim} \
   -r ${dcuberef_sim} \
   ${dcubemon_sim}
 echo "art-result: $? dcube_sim"
 
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_lastref} \
   -c ${dcubecfg_sim} \
   -r ${lastref_dir}/${dcubemon_sim} \
   ${dcubemon_sim}
 echo "art-result: $? dcube_sim_last"

 run Digi_tf.py \
   --conditionsTag default:$conditionsTag \
   --digiSeedOffset1 100 --digiSeedOffset2 100 \
   --inputHITSFile $hits \
   --maxEvents -1 \
   --outputRDOFile $rdo \
   --preInclude 'HITtoRDO:Campaigns.MC23NoPileUp' \
   --postInclude 'PyJobTransforms.UseFrontier' 
 echo "art-result: $? digi"

 run RunRDOAnalysis.py \
   -i $rdo \
   Pixel SCT TRT
 echo "art-result: $? RDOAnalysis"

 # Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
 run Reco_tf.py \
   --inputRDOFile    $rdo \
   --outputAODFile   $aod \
   --conditionsTag   default:$conditionsTag \
   --steering        doRAWtoALL \
   --checkEventCount False \
   --ignoreErrors    True \
   --maxEvents       -1
 rec_tf_exit_code=$?
 echo "art-result: $rec_tf_exit_code reco"

 runIDPVM.py \
   --filesInput $aod \
   --outputFile ${dcubemon_rec} \
   --doHitLevelPlots \
   --doExpertPlots
 idpvm_tf_exit_code=$?
 echo "art-result: $idpvm_tf_exit_code idpvm"
 
 if [ $rec_tf_exit_code -eq 0 ]  ;then
 
   echo "compare with a fixed reference"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_fixref} \
     -c ${dcubeshiftercfg_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}
   echo "art-result: $? dcube_rec"
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_lastref} \
     -c ${dcubeshiftercfg_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}
   echo "art-result: $? dcube_rec_last"

   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_fixref} \
     -c ${dcubeexpertcfg_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}

   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_lastref} \
     -c ${dcubeexpertcfg_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}

   echo "compare with a fixed reference for RDOAnalysis"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rdo_fixref} \
     -c ${dcubecfg_rdo} \
     -r ${dcuberef_rdo} \
     ${dcubemon_rdo}
   echo "art-result: $? dcube_rdo"
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rdo_lastref} \
     -c ${dcubecfg_rdo} \
     -r ${lastref_dir}/${dcubemon_rdo} \
     ${dcubemon_rdo}
   echo "art-result: $? dcube_rdo_last"
 fi

fi
