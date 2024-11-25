#!/bin/bash
# art-description: Standard test for MC23a zprime for IDTIDE
# art-input: mc23_13p6TeV:mc23_13p6TeV.801271.Py8EG_A14NNPDF23LO_flatpT_Zprime.merge.HITS.e8514_e8528_s4159_s4114
# art-input-nfiles: 1
# art-cores: 4
# art-memory: 4096
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: art_core_0
# art-output: dcube*
# art-html: dcube_idtide_last

relname="r24.0.65"

lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_idtide.xml"
dcubeRef=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_zprime_tide.root

rdo=physval.RDO.root
aod=physval.AOD.root
idtide=DAOD_TIDE.pool.root

conditionsTag=OFLCOND-MC23-SDR-RUN3-07

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

      echo "compare with fixed reference"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter \
	   -c ${dcubeXml} \
	   -r ${dcubeRef} \
	   physval.root
      echo "art-result: $? shifter_plots"

      echo "compare with last build"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter_last \
	   -c ${dcubeXml} \
	   -r last_results/physval.root \
	   physval.root
      echo "art-result: $? shifter_plots_last"

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

    # Digi for MC23d HITS inputs
    Digi_tf.py \
	--conditionsTag default:$conditionsTag \
	--digiSeedOffset1 100 --digiSeedOffset2 100 \
	--inputHITSFile=${ArtInFile} \
	--maxEvents -1 \
	--outputRDOFile $rdo \
	--preInclude 'HITtoRDO:Campaigns.MC23dNoPileUp' \
	--postInclude 'PyJobTransforms.UseFrontier'
    echo "art-result: $? digi"

    # Reco
    Reco_tf.py \
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

    #run IDPVM for IDTIDE derivation
    run runIDPVM.py \
	--doIDTIDE --doTracksInJets --doTracksInBJets \
	--filesInput $idtide \
	--outputFile physval.ntuple.root
    idpvm_tf_exit_code=$?
    echo "art-result: $idpvm_tf_exit_code idpvm"

    if [ $rec_tf_exit_code -ne 0 ]  ;then
      success_run=$rec_tf_exit_code
    fi
    ls -lR
    ;;
esac
