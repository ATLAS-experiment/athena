#!/bin/bash
# art-description: Standard test for MC23a zprime for IDTIDE
# art-input: mc23_13p6TeV:mc23_13p6TeV.801271.Py8EG_A14NNPDF23LO_flatpT_Zprime.merge.HITS.e8514_e8528_s4159_s4114
# art-input-nfiles: 10
# art-cores: 8
# art-memory: 4096
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_idtide_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

relname="r24.0.109"

lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_idtide.xml"
dcubeRef=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_zprime_tide.root

rdo=physval.RDO.root
aod=physval.AOD.root
idtide=physval.DAOD_TIDE.root


case $ArtProcess in
  "start")
    echo "List of files = " ${ArtInFile}
    ;;
  "end")
    if ls art_core_*/${idtide} >/dev/null 2>&1 ; then

      echo "Merging physval.DAOD_TIDE.root"
      hadd ${idtide} art_core_*/${idtide}

      #run IDPVM for IDTIDE derivation
      run runIDPVM.py --doIDTIDE --doTracksInJets --doTracksInBJets --filesInput $idtide --outputFile physval_idtide.ntuple.root

      echo "download latest result"
      run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
      run ls -la "$lastref_dir"

      echo "compare with fixed reference"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_idtide \
        -c ${dcubeXml} \
        -r ${dcubeRef} \
        physval_idtide.ntuple.root
      echo "art-result: $? shifter_plots_idtide"
      
      echo "compare with last build"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_idtide_last \
        -c ${dcubeXml} \
        -r ${lastref_dir}/physval_idtide.ntuple.root \
        physval_idtide.ntuple.root
      echo "art-result: $? shifter_plots_idtide_last"

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

    conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

    # Digi for MC23d HITS inputs
    run Digi_tf.py \
        --conditionsTag default:$conditionsTag \
        --digiSeedOffset1 100 --digiSeedOffset2 100 \
        --inputHITSFile=$x \
        --maxEvents -1 \
        --outputRDOFile $rdo \
        --preInclude 'HITtoRDO:Campaigns.MC23dNoPileUp' \
        --postInclude 'PyJobTransforms.UseFrontier'

    digi_tf_exit_code=$?
    echo "art-result: $digi_tf_exit_code digi"

    if [ $digi_tf_exit_code -eq 0 ]  ;then

      # Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
      run Reco_tf.py \
          --inputRDOFile    $rdo \
          --outputAODFile   $aod \
          --outputDAOD_IDTIDEFile $idtide \
          --conditionsTag   default:$conditionsTag \
          --steering        doRAWtoALL \
          --checkEventCount False \
          --ignoreErrors    True \
          --maxEvents       -1

      rec_tf_exit_code=$?
      echo "art-result: $rec_tf_exit_code reco"

    fi 
    ;;

esac