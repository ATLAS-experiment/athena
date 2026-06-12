#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration



RUN_TAG=$1

GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_${RUN_TAG})")

RunWorkflowTests_Run3.py --CI \
        -s -w FullSim \
        -a s4454 \
        --threads 4 \
        -e "--maxEvents 50 --geometrySQLite True --geometrySQLiteFullPath ${GEOMODEL_DB_FILE}" \
        --run-only
  