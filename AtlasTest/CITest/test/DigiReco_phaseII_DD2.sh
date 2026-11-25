#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

RUN_TAG=$1
export ATHENA_CORE_NUMBER=1

HITS_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(\",\".join(MuonPhaseIITestDefaults.HITS_PG_${RUN_TAG}))")

Reco_tf.py \
  --conditionsTag "default:COND-MC21-SDR-RUN4-06" \
  --preInclude "all:Campaigns.MC23PhaseIINoPileUp" \
  --geometrySQLite True \
  --geometryVersion "default:ATLAS-DD2-P2-RUN4-05-01-00" \
  --preExec "RAWtoALL:flags.Acts.TrackingGeometry.UseBlueprint = True" \
  --inputHITSFile ${HITS_FILE} \
  --outputRDOFile RUN4.RDO.pool.root \
  --outputAODFile RUN4.AOD.pool.root \
  --imf="False" \
  --maxEvents 5 \
  --ignorePatterns 'ERROR ROOT PCM' \
  --multithreaded True

