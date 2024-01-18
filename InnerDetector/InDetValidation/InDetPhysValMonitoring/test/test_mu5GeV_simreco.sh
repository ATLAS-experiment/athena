#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single mu 5GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422033.ParticleGun_single_mu_Pt5.merge.EVNT.e7967_e5984_tid20254920_00
# art-input-nfiles: 1
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: SiHitValid*.root
# art-output: *Analysis*.root
# art-output: *.xml 
# art-output: dcube*

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

# Following specify DCube output directories. Set empty to disable.
dcube_sim_fixref="dcube_sim"
dcube_sim_lastref="dcube_sim_last"
dcube_rdo_fixref="dcube_rdo"
dcube_rdo_lastref="dcube_rdo_last"
dcube_rec_fixref="dcube_shifter"
dcube_rec_lastref="dcube_shifter_last"
dcube_rec_expert_fixref="dcube_expert"
dcube_rec_expert_lastref="dcube_expert_last"

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
name="run2"
relname="r24"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  relname="r23"
fi
script="`basename \"$0\"`"
hits=physval.HITS.root
dcubemon_sim=SiHitValid.root
dcubemon_rec=physval.ntuple.root
dcubemon_rdo=RDOAnalysis.root
dcubecfg_sim=$artdata/InDetPhysValMonitoring/dcube/config/run2_SiHitValid.xml
dcuberef_sim=$artdata/InDetPhysValMonitoring/ReferenceHistograms/SiHitValid_mu_5GeV_simreco_${relname}.root
dcubecfg_rdo=$artdata/InDetPhysValMonitoring/dcube/config/run2_RDOAnalysis.xml
dcuberef_rdo=$artdata/InDetPhysValMonitoring/ReferenceHistograms/RDOAnalysis_mu_5GeV_simreco_${relnam}.root
dcubecfgshifter_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_baseline.xml
dcubecfgexpert_rec=$artdata/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_expert.xml
dcuberef_rec=$artdata/InDetPhysValMonitoring/ReferenceHistograms/physval_mu_5GeV_simreco_${relname}.root
art_dcube=$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py

lastref_dir=last_results

# Sim step (based on PanDA job 3777178576 which made HITS.12860054._032508.pool.root.1, which is the input for AMI config q221):
 run Sim_tf.py \
    --CA \
    --inputEVNTFile   ${ArtInFile} \
    --outputHITSFile  "$hits" \
    --skipEvents      0 \
    --maxEvents       10000 \
    --randomSeed      24304 \
    --simulator       FullG4MT_QS \
    --conditionsTag   'OFLCOND-MC23-SDR-RUN3-01' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --postInclude     'PyJobTransforms.TransformUtils.UseFrontier' 'HitAnalysis.PostIncludes.IDHitAnalysis'
sim_tf_exit_code=$?
echo "art-result: $sim_tf_exit_code sim"

if [ $sim_tf_exit_code -eq 0 ]  ;then

 echo "download latest result"
 run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
 run ls -la "$lastref_dir"
 # DCube Sim hit plots
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_fixref} \
   -c ${dcubecfg_sim} \
   -r ${dcuberef_sim} \
   ${dcubemon_sim}
 
 $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
   -p -x ${dcube_sim_lastref} \
   -c ${dcubecfg_sim} \
   -r ${lastref_dir}/${dcubemon_sim} \
   ${dcubemon_sim}
 echo "art-result: $? dcube_sim_last"
 
 run Digi_tf.py \
   --CA \
   --conditionsTag default:OFLCOND-MC23-SDR-RUN3-01 \
   --digiSeedOffset1 100 --digiSeedOffset2 100 \
   --geometryVersion "default:ATLAS-R3S-2021-03-02-00" \
   --inputHITSFile $hits \
   --maxEvents -1 \
   --outputRDOFile output.RDO.root \
   --preInclude 'HITtoRDO:Campaigns.MC23NoPileUp' \
   --postInclude 'PyJobTransforms.UseFrontier' 
 echo "art-result: $? digi"

 run RunRDOAnalysis.py \
    -i output.RDO.root \
    Pixel SCT
 echo "art-result: $? RDOAnalysis"

 # Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
 run Reco_tf.py \
   --CA \
   --inputRDOFile   output.RDO.root \
   --outputAODFile   physval.AOD.root \
   --conditionsTag   'default:OFLCOND-MC23-SDR-RUN3-01' \
   --steering        doRAWtoALL \
   --checkEventCount False \
   --ignoreErrors    True \
   --maxEvents       -1
 rec_tf_exit_code=$?
 echo "art-result: $rec_tf_exit_code reco"
 
 runIDPVM.py \
   --filesInput physval.AOD.root \
   --outputFile ${dcubemon_rec} \
   --doHitLevelPlots \
   --doExpertPlots
 idpvm_tf_exit_code=$?
 echo "art-result: $idpvm_tf_exit_code idpvm"
 
 if [ $rec_tf_exit_code -eq 0 ]  ;then
 
   echo "compare with a fixed R22"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_fixref} \
     -c ${dcubecfgshifter_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_lastref} \
     -c ${dcubecfgshifter_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}
   echo "art-result: $? dcube_rec_last"

   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_fixref} \
     -c ${dcubecfgexpert_rec} \
     -r ${dcuberef_rec} \
     ${dcubemon_rec}
   
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rec_expert_lastref} \
     -c ${dcubecfgexpert_rec} \
     -r ${lastref_dir}/${dcubemon_rec} \
     ${dcubemon_rec}

   echo "compare with a fixed R22 for PixelRDOAnalysis"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rdo_fixref} \
     -c ${dcubecfg_rdo} \
     -r ${dcuberef_rdo} \
     ${dcubemon_rdo}
   
   echo "compare with last build"
   $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
     -p -x ${dcube_rdo_lastref} \
     -c ${dcubecfg_rdo} \
     -r ${lastref_dir}/${dcubemon_rdo} \
     ${dcubemon_rdo}
   echo "art-result: $? dcube_rdo_last"

 fi

fi
