#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

RUN_TAG=$1

GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_${RUN_TAG})")

export ATHENA_PROC_NUMBER=1
export ATHENA_CORE_NUMBER=1


GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_${RUN_TAG})")
HITS_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(\",\".join(MuonPhaseIITestDefaults.HITS_PG_${RUN_TAG}))")

Digi_tf.py \
        --inputHITSFile ${HITS_FILE} \
        --multithreaded True \
        --geometrySQLite True \
	--geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
        --digiSeedOffset1 170 \
        --digiSeedOffset2 170 \
        --digiSteeringConf 'StandardSignalOnlyTruth' \
        --jobNumber 568 \
        --outputRDOFile myRDO.pool.root \
        --skipEvents 0 \
        --maxEvents 5  \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --ignorePatterns 'ERROR ROOT PCM' \
        --preExec='from MuonConfig.MuonConfigUtils import configureDefaultTags;configureDefaultTags(flags);from Campaigns.PhaseII import MC23PhaseIIPileUpMC21a; from Campaigns.MC23 import MC23d;from AthenaConfiguration.Enums import LHCPeriod;(MC23d(flags) if flags.GeoModel.Run == LHCPeriod.Run3 else MC23PhaseIIPileUpMC21a(flags));'
