#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART Run 4 configuration, ITK only recontruction, acts activated

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

ArtProcess=$1
ArtInFile=$2
dcuberef_sim=$3
dcuberef_rdo=$4
dcuberef_rec=$5

echo "ArtProcess: $ArtProcess"

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

sim_tf_exit_code=0
rdoana_tf_exit_code=0
idpvm_tf_exit_code=0

# Don't run if dcube config not found
if [ -z "$dcubeshiftercfg_rec" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi


case $ArtProcess in
  "start")
    echo "List of files = " ${ArtInFile}
    ;;
  "end")
    if [ $sim_tf_exit_code -eq 0 ]  ;then
        echo "download latest result" # download results if 1st step was successful
        run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
        run ls -la "$lastref_dir"

        echo "Merging HitValid.root"
        hadd ${dcubemon_sim} art_core_*/HitValid.root

        # DCube Sim hit plots
        $art_dcube \
            -p -x ${dcube_sim_fixref} \
            -c ${dcubecfg_sim} \
            -r ${dcuberef_sim} \
            ${dcubemon_sim}
        echo "art-result: $? dcube_sim"

        $art_dcube \
            -p -x ${dcube_sim_lastref} \
            -c ${dcubecfg_sim} \
            -r ${lastref_dir}/${dcubemon_sim} \
            ${dcubemon_sim}
        echo "art-result: $? dcube_sim_last"
    fi

    if [ $rdoana_tf_exit_code -eq 0 ] ;then

        echo "Merging RDOAnalysis.root"
        hadd ${dcubemon_rdo} art_core_*/RDOAnalysis.root

        echo "compare with a fixed reference for RDOAnalysis"
        $art_dcube \
            -p -x ${dcube_rdo_fixref} \
            -c ${dcubecfg_rdo} \
            -r ${dcuberef_rdo} \
            ${dcubemon_rdo}
        echo "art-result: $? dcube_rdo"

        echo "compare with last build"
        $art_dcube \
            -p -x ${dcube_rdo_lastref} \
            -c ${dcubecfg_rdo} \
            -r ${lastref_dir}/${dcubemon_rdo} \
            ${dcubemon_rdo}
        echo "art-result: $? dcube_rdo_last"
    fi

    if [ $idpvm_tf_exit_code -eq 0 ]  ;then
        echo "Merging physval.ntuple.root"
        hadd  ${dcubemon_rec} art_core_*/physval.ntuple.root
        echo "postprocess"
        postProcessIDPVMHistos ${dcubemon_rec}

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

        echo "art-result: $? dcube_rec_last"
    fi
    ;;
  *)
    # Setup for multi-cores
    echo "Test $ArtProcess"
    mkdir "art_core_${ArtProcess}"
    cd "art_core_${ArtProcess}"
    IFS=',' read -r -a file <<< "${ArtInFile}"
    file=${file[${ArtProcess}]}
    x="../$file"
    echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC} and ATHENA_PROC_NUMBER=${ATHENA_PROC_NUMBER}"
    unset  ATHENA_NUM_PROC
    unset  ATHENA_PROC_NUMBER

    geotag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
    conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

    # Run Simulation
    run Sim_tf.py \
        --inputEVNTFile   $x \
        --outputHITSFile  "$hits" \
        --skipEvents      0 \
        --maxEvents       1000 \
        --randomSeed      24304 \
        --simulator       FullG4MT_QS \
        --conditionsTag   default:$conditionsTag \
        --geometryVersion default:$geotag \
        --preExec         "default:flags.Output.HISTFileName='${dcubemon_sim}';" \
        --preInclude      'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
        --postInclude     'PyJobTransforms.TransformUtils.UseFrontier' 'HitAnalysis.PostIncludes.IDHitAnalysis'

    sim_tf_exit_code=$?
    echo "art-result: $sim_tf_exit_code sim"

    if [ $sim_tf_exit_code -eq 0 ]  ;then

        # Run digitization + RDOAnalysis
        run Digi_tf.py \
            --conditionsTag default:$conditionsTag \
            --digiSeedOffset1 100 --digiSeedOffset2 100 \
            --inputHITSFile $hits \
            --maxEvents -1 \
            --outputRDOFile $rdo \
            --preInclude 'HITtoRDO:Campaigns.MC23aNoPileUp' \
            --postInclude 'PyJobTransforms.UseFrontier' 

        digi_tf_exit_code=$?
        echo "art-result: $digi_tf_exit_code digi"

        run RunRDOAnalysis.py \
            -i $rdo \
            Pixel SCT TRT
        rdoana_tf_exit_code=$?
        echo "art-result: $rdoana_tf_exit_code RDOAnalysis"
    fi


    if [ $digi_tf_exit_code -eq 0 ]  ;then

        # Run Reconstruction
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

        # Run IDPVM
        runIDPVM.py \
            --filesInput $aod \
            --outputFile ${dcubemon_rec} \
            --doHitLevelPlots \
            --doExpertPlots

        idpvm_tf_exit_code=$?
        echo "art-result: $idpvm_tf_exit_code idpvm"
    fi
    ;;
esac
