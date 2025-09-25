#!/bin/bash

FILE_ARG=${1}
GEO_ARG=${2}

GEOTAG=""
if [ -n "${GEO_ARG}" ]; then
    ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.${GEO_ARG})")
    GEOTAG="--geoTag ${ATLAS_GEO_TAG}"
fi
IN_FILE=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.${FILE_ARG}[0])")

"python -m MuonCondTest.MdtCablingTester -i  ${IN_FILE} ${GEOTAG}"
python -m MuonCondTest.MdtCablingTester -i ${IN_FILE} ${GEOTAG}