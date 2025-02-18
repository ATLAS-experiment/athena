#!/bin/sh
#
# art-description: ART Monitoring Tool for gamma Validation
#
# art-type: grid
# art-input: mc16_13TeV.423001.ParticleGun_single_photon_egammaET.recon.RDO.e3566_s3112_r12663
# art-input-nfiles: 30
# art-cores: 4
# art-include: main/Athena
# art-include: 24.0/Athena
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

	checkFile.py Nightly_AOD.pool.root > checkFile_Nightly_gamma.txt

	echo  "art-result: $? checks_files"

	runegammaMonitoring.py -p 'gamma'

	echo  "art-result: $? athena_job"

	EgammaARTmonitoring_plotsMaker.py /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/ref_24/Nightly-monitoring_gamma.hist.root Nightly-monitoring.hist.root gamma

	echo  "art-result: $? final_comparison"

	$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p -x dcube -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/DCube_Config/gamma.xml -r /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/egammaValidation/Nightly_Files/ref_24/Nightly-monitoring_gamma.hist.root  Nightly-monitoring.hist.root
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

	Reco_tf.py --CA --inputRDOFile=$x --outputAODFile=Nightly_AOD.pool.root --maxEvents=2000 --autoConfiguration="everything" --conditionsTag="OFLCOND-MC16-SDR-RUN2-09" --preInclude egammaConfig.ConfigurationHelpers.egammaOnlyFromRaw --postInclude egammaValidation.egammaArtSpecialContent.egammaArtSpecialContent

	echo  "art-result: $? reconstruction"

	;;
esac
