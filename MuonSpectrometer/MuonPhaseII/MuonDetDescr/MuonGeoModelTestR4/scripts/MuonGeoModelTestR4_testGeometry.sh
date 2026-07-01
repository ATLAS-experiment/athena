#!/bin/bash

EXTRA_ARG="${2}"

EXTRA_OPT=""
if [ ${EXTRA_ARG} == "DATA "]; then
    export IN_FILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.BS[0])")
    EXTRA_OPT="--inputFile ${IN_FILE}"
fi

if [ ${EXTRA_ARG} == "MTECH "]; then
    export GEOMODEL_DB_FILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.GEODB_MTECH)")
    EXTRA_OPT="--geoModelFile ${GEOMODEL_DB_FILE}"
fi

python -m MuonGeoModelTestR4.testGeoModel \
        --chambers STL1A1 BML1A3 T1E1A01 MML1A1 \
        --defaultGeoFile ${1} \
        ${EXTRA_OPT}
