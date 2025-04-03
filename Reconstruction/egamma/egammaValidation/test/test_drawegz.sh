#!/bin/sh
#
# art-description: ART Monitoring Tool for electron Validation
#
# art-type: grid
# art-input: user.gunal.data22_13p6TeV.00439911.physics_Main.merge.DRAW_EGZ.f1310_m2151_der1689237472
# art-input-nfiles: 5
# art-cores: 8
# art-include: main/Athena
# art-output: *.hist.root
# art-output: *.txt
# art-output: *.png
# art-output: log.*
# art-output: dcube

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

	checkFile.py Nightly_AOD.pool.root > checkFile_Nightly_dataZ.txt

	echo  "art-result: $? checks_files"

	runegammaMonitoring.py -p 'dataZ'

	echo  "art-result: $? athena_job"

	$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p -x dcube -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/DCube_Config/dataZee.xml -r /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/Nightly-monitoring_data22DRAWEGZ_run439911.f1310_m2151_rel24_0_12.hist.root Nightly-monitoring.hist.root

	echo  "art-result: $? plot"

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

	Reco_tf.py --CA --inputBSFile=$x --outputAODFile=Nightly_AOD.pool.root --maxEvents=4000 --autoConfiguration="everything" --geometryVersion="ATLAS-R3S-2021-03-02-00" --conditionsTag="CONDBR2-BLKPA-2022-12" --preInclude egammaConfig.ConfigurationHelpers.egammaOnlyFromRaw --postInclude egammaValidation.egammaArtSpecialContent.egammaArtSpecialContent

	echo  "art-result: $? reconstruction"

	;;
esac
