#!/bin/bash

export GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_TOROID);")
RunGeantinoMaterialTrackProduction.py \
        --geoModelSqLiteFile ${GEOMODEL_DB_FILE} \
        --detectors toroid \
        --maxEvents 1