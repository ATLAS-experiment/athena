#!/bin/sh
#
# art-description: ART Monitoring Tool for electron Validation, phase II, no pileup
#
# art-type: grid
# art-input: mc21_14TeV.901967.PG_single_epm_egammaET_etaFlatnp0_25.recon.RDO.e8481_s4264_r15317
# art-input-nfiles: 60
# art-cores: 4
# art-include: main/Athena
# art-output: *.hist.root
# art-output: *.txt
# art-output: *.png
# art-output: log.*
# art-output: dcube

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

echo "ArtProcess: $ArtProcess"

case $ArtProcess in
    
    "start")
	echo "Starting"
	echo "List of files = " ${ArtInFile}
	;;

    "end")
	echo "Ending"
	
	echo "Merging AODs"
        echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC}"
        unset  ATHENA_NUM_PROC

	AODMerge_tf.py --CA --inputAODFile=art_core_*/Nightly_AOD.pool.root --outputAOD_MRGFile=Nightly_AOD.pool.root

	echo  "art-result: $? AODMerge"

	set +e

	checkFile.py Nightly_AOD.pool.root > checkFile_Nightly.txt

	echo  "art-result: $? checks_files"

	runegammaMonitoring.py -p 'electron'

	echo  "art-result: $? athena_job"

	EgammaARTmonitoring_plotsMaker.py /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/ref_main/Nightly-monitoring_electron.hist.root Nightly-monitoring.hist.root electron

	echo  "art-result: $? final_comparison"

	## dcube
	$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p -x dcube -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/DCube_Config/electron.xml -r /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/ref_main/Nightly-monitoring_electron_cen_Run4.hist.root Nightly-monitoring.hist.root
	#echo  "art-result: $? plot"

	;;

    *)
	echo "Test $ArtProcess"

	mkdir "art_core_${ArtProcess}"
	cd "art_core_${ArtProcess}"

	IFS=',' read -r -a file <<< "${ArtInFile}"
	file=${file[${ArtProcess}]}
	x="../$file"

	echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC}"
	unset  ATHENA_NUM_PROC

	Reco_tf.py --CA --inputRDOFile=$x --outputAODFile=Nightly_AOD.pool.root --maxEvents=1000 --autoConfiguration="everything" --conditionsTag="${conditions}" --preInclude egammaConfig.ConfigurationHelpers.egammaOnlyFromRaw --postInclude egammaValidation.egammaArtSpecialContent.egammaArtSpecialContent --preExec "flags.Calo.Noise.fixedLumiForNoise=68.965" --postExec "from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags,\"/LAR/NoiseOfl/CellNoise\",\"LARNoiseOflCellNoise-mu200\",db=\"sqlite://;schema=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/dbNoisePhaseII/CellNoise-mu200Formu0Sample.db;dbname=OFLP200\"))"

	echo  "art-result: $? reconstruction"

	;;
esac
