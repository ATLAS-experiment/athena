#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

echo "--------------------------------------------"
echo "Running ACTS configuration tests"
echo "--------------------------------------------"

checkCollectionOnFile() {
    local trackCollections=("$@")
    checkxAOD.py AOD.pool.root >& collections.txt
    echo " * checking AOD file content for collections:"
    for var in "${trackCollections[@]}"; do
	echo "   - checking collection: "${var//\"/}"TrackParticles"
	grep -e " ${var//\"/}TrackParticles " collections.txt >& tmp.log
	res=$?
	if [ $res != 0 ]; then
	    return ${res}
	fi
    done
    return 0
}

checkIdpvmOnFile() {
    trackCollections="$@"
    checkCollectionOnFile \
	${trackCollections}
    grep_rc=$?
    if [ $grep_rc != 0 ]; then
	echo "Collection missing in AOD file"
	checkxAOD.py AOD.pool.root
	return ${grep_rc}
    fi
    echo " * running IDPVM on AOD file, checking the following collections: ${trackCollections}"
    runIDPVM.py \
	--filesInput AOD.pool.root \
	--outputFile idpvm.root \
	--doExpertPlots \
	--OnlyTrackingPreInclude \
	--doActs \
	--doTechnicalEfficiency \
	--validateExtraTrackCollections ${trackCollections} >& idpvm.log
    idpvm_rc=$?
    if [ $idpvm_rc != 0 ]; then
	cat idpvm.log
    fi
    return $idpvm_rc
}

failed_tests=()

# Few flags
activate_all_flags="flags.Acts.doLargeRadius=True; \
		    flags.Acts.doLowPt=True; \
       		    flags.Tracking.ITkActsPass.storeSeparateContainer=True; \
        	    flags.Tracking.ITkActsLargeRadiusPass.storeSeparateContainer=True; \
        	    flags.Tracking.ITkActsConversionPass.storeSeparateContainer=True; \
		    flags.Tracking.ITkActsLowPtPass.storeSeparateContainer=True; \
		    flags.Tracking.ITkActsPass.storeTrackSeeds=True; \
		    flags.Tracking.ITkActsLargeRadiusPass.storeTrackSeeds=True; \
        	    flags.Tracking.ITkActsConversionPass.storeTrackSeeds=True; \
		    flags.Tracking.ITkActsLowPtPass.storeTrackSeeds=True; \
        	    flags.Tracking.ITkActsPass.storeSiSPSeededTracks=True; \
        	    flags.Tracking.ITkActsLargeRadiusPass.storeSiSPSeededTracks=True; \
		    flags.Tracking.ITkActsConversionPass.storeSiSPSeededTracks=True; \
        	    flags.Tracking.ITkActsLowPtPass.storeSiSPSeededTracks=True; \
        	    flags.Acts.EDM.PersistifyTracks=True; \
        	    flags.Acts.EDM.PersistifySpacePoints=True; \
		    flags.Detector.EnableCalo=True; \
		    flags.Tracking.writeExtendedSi_PRDInfo=True; \
		    flags.Acts.storeTrackStateInfo=True;"
		   
activate_all_collections="\"InDet\" \
        \"InDetActsConversion\" \
        \"InDetLargeD0\" \
        \"InDetActsLowPt\" \
        \"SiSPSeedSegmentsActsConversionStrip\" \
        \"SiSPSeedSegmentsActsLargeRadiusStrip\" \
        \"SiSPSeedSegmentsActsLowPtPixel\" \
        \"SiSPSeededTracksActsLargeRadius\" \
        \"SiSPSeededTracksActsConversion\" \
        \"SiSPSeededTracksActsLowPt\" \
	"

# Activate everything!
echo "Running configuration 1: activate all - ACTS chain with production configuration"
source ActsMainConfiguration.sh \
       "${activate_all_flags} \
       " \
       "" \
       >& ActsConfiguration1.log

reco_rc=$?
if [ $reco_rc != 0 ]; then
    echo " - FAILURE (RECO)"
    failed_tests+=("Configuration 1")
    cat log.RAWtoALL
else
    checkIdpvmOnFile \
	${activate_all_collections}
    idpvm_rc=$?
    cp idpvm.log idpvm1.log
    if [ $idpvm_rc != 0 ]; then
	echo " - FAILURE (IDPVM)"
	failed_tests+=("Configuration 1")
    fi
fi


# Run on Heavy Ions
echo "Running Configuration 2: Heavy Ion"
source ActsHeavyConfiguration.sh \
       "flags.Tracking.ITkActsHeavyIonPass.storeSeparateContainer=True; \
	flags.Tracking.ITkActsHeavyIonPass.storeTrackSeeds=True; \
	flags.Tracking.ITkActsHeavyIonPass.storeSiSPSeededTracks=True; \
	flags.Tracking.writeExtendedSi_PRDInfo=True; \
       "  >& ActsConfiguration2.log

reco_rc=$?
if [ $reco_rc != 0 ]; then
    echo " - FAILURE (RECO)"
    failed_tests+=("Configuration 2")
    cat log.RAWtoALL
else
    checkIdpvmOnFile \
        "InDet" \
        "InDetActsHeavyIon" \
        "SiSPSeedSegmentsActsHeavyIonPixel" \
	"SiSPSeedSegmentsActsHeavyIonStrip" \
        "SiSPSeededTracksActsHeavyIon"
    idpvm_rc=$?
    cp idpvm.log idpvm2.log
    if [ $idpvm_rc != 0 ]; then
        echo " - FAILURE (IDPVM)"
        failed_tests+=("Configuration 2")
    fi
fi

echo "--------------------------------------------"
if [ ${#failed_tests[@]} -eq 0 ]; then
    exit 0
else
    echo "List of failed tests:"
    for element in "${failed_tests[@]}"; do
	echo "- $element"
    done
fi
exit 1


