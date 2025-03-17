#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# single mu HITS
input_hits=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900495.PG_single_muonpm_Pt10_etaFlatnp0_43.simul.HITS.e8481_s4149/*
n_events=1000

checkCollectionOnFile() {
    local collections=("$@")
    checkxAOD.py RDO.pool.root >& storedCollections.txt
    for var in "${collections[@]}"; do
	echo "   - checking collection: "${var//\"/}
	grep -e " ${var//\"/} " storedCollections.txt >& tmp.log
	res=$?
	if [ $res != 0 ]; then
	    return ${res}
	fi
    done
    return 0
}

export ATHENA_CORE_NUMBER=8
# Run HITS -> RDO with persistified SDO and SiHit
Reco_tf.py \
    --inputHITSFile ${input_hits} \
    --outputRDOFile RDO.pool.root \
    --maxEvents ${n_events} \
    --autoConfiguration "everything" \
    --conditionsTag "OFLCOND-MC15c-SDR-14-05" \
    --geometryVersion "ATLAS-P2-RUN4-03-00-00" \
    --preInclude "Campaigns.PhaseIINoPileUp,InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "PyJobTransforms.UseFrontier" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg; \
    	        cfg.merge( OutputStreamCfg(ConfigFlags, \"RDO\", [\"SiHitCollection#*\"]) );" \
    --steering "doRAWtoALL" \
    --multithreaded

rc=$?
if [ $rc != 0 ]; then
    exit 1
fi
    
# Check SiHit collections are in the RDO file
checkCollectionOnFile "ITkStripHits" "ITkPixelHits" "ITkStripSDO_Map" "ITkPixelSDO_Map"

rc=$?
if [ $rc != 0 ]; then
    checkxAOD.py RDO.pool.root
    exit 1
fi

# Run RDO -> AOD with ACTS and asking for extra PRD decorations
Reco_tf.py \
    --inputRDOFile RDO.pool.root \
    --outputAODFile AOD.pool.root \
    --maxEvents ${n_events} \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.doTIDE_AmbiTrackMonitoring=True;" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --multithreaded

rc=$?
if [ $rc != 0 ]; then
    exit 1
fi

exit 0

