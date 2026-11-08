#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

export ATHENA_CORE_NUMBER=1

GEOMODEL_DB_VERSION="ATLAS-DD2-P2-RUN4-05-01-00"
CONDITIONS_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Sim_tf.py \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8481/EVNT.33964680._002197.pool.root.1" \
    --outputHITSFile "myHITS.pool.root" \
    --imf "False" \
    --geometryVersion ${GEOMODEL_DB_VERSION} \
    --conditionsTag ${CONDITIONS_TAG} \
    --geometrySQLite "True" \
    --preInclude "EVNTtoHITS:Campaigns.MC23PhaseIISimulation" \
    --simulator "FullG4MT_QS" \
    --maxEvents "5" \
    --multithreaded "True"
