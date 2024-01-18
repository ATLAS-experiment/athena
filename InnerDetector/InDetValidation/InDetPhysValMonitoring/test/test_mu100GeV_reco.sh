#!/bin/bash
# art-description: art job for InDetPhysValMonitoring, Single muon 100GeV
# art-type: grid
# art-input: user.keli:user.keli.mc16_13TeV.422036.ParticleGun_single_mu_Pt100GeV_Rel22073
# art-input-nfiles: 10
# art-cores: 4
# art-memory: 4096
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: *.xml 
# art-output: art_core_0
# art-output: dcube*
# art-html: dcube_last

#RDO is made at rel 22.0.73
#reference plots are made at rel 22.0.73

set -x

echo "ArtProcess: $ArtProcess"
lastref_dir=last_results
script="`basename \"$0\"`"
success_run=0

case $ArtProcess in
  "start")
    echo "Starting"
    echo "List of files = " ${ArtInFile}
    ;;
  "end")
    echo "Ending"
    if [ ${success_run} -eq 0 ]  ;then
      echo "download latest result"
      art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
      ls -la "$lastref_dir"
      echo "Merging physval.root"
      hadd  physval.root art_core_*/physval.ntuple.root
      echo "postprocess"
      postProcessIDPVMHistos physval.root

      dcubeShifterXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_baseline.xml"
      dcubeExpertXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_mc_expert.xml"
      dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_mu100GeV_reco_r24.root"
      if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
        dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_mu100GeV_reco_r23.root"
      fi
      echo "compare with R23.0.23 or 24.0.1"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter \
	   -c ${dcubeShifterXml} \
	   -r ${dcubeRef} \
	   physval.root
      echo "art-result: $? shifter_plots"

      echo "compare with last build"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter_last \
	   -c ${dcubeShifterXml} \
	   -r last_results/physval.root \
	   physval.root
      echo "art-result: $? shifter_plots_last"

      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_expert\
	   -c ${dcubeExpertXml} \
	   -r ${dcubeRef} \
	   physval.root

      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_expert_last \
	   -c ${dcubeExpertXml} \
	   -r last_results/physval.root \
	   physval.root
    else
      echo "reco failed"
    fi
    ;;
  *)
    echo "Test $ArtProcess"
    mkdir "art_core_${ArtProcess}"
    cd "art_core_${ArtProcess}"
    IFS=',' read -r -a file <<< "${ArtInFile}"
    file=${file[${ArtProcess}]}
    x="../$file"
    echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC} and ATHENA_PROC_NUMBER=${ATHENA_PROC_NUMBER}"
    unset  ATHENA_NUM_PROC
    unset  ATHENA_PROC_NUMBER

    Reco_tf.py \
      --CA \
      --inputRDOFile $x \
      --outputAODFile   physval.AOD.root \
      --conditionsTag   'default:OFLCOND-MC23-SDR-RUN3-01' \
      --steering        doRAWtoALL \
      --checkEventCount False \
      --ignoreErrors    True \
      --maxEvents       -1 
    rec_tf_exit_code=$?
    echo "art-result: $rec_tf_exit_code reco_${file}"

    runIDPVM.py \
      --filesInput physval.AOD.root \
      --outputFile physval.ntuple.root \
      --doHitLevelPlots \
      --doExpertPlots
    idpvm_tf_exit_code=$?
    echo "art-result: $idpvm_tf_exit_code idpvm"

    if [ $rec_tf_exit_code -ne 0 ]  ;then
       success_run=$rec_tf_exit_code 
    fi
    ls -lR
    ;;
esac
