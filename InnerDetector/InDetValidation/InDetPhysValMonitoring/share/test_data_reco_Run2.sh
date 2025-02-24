#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART jobs with Run 2 Data Reco config

inputBS=$1
dcubeRef=$2
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN2_DATA)")
geotag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometr
yTags; print(defaultGeometryTags.RUN2)")

script=test_data_reco.sh

"$script" ${inputBS} ${dcubeRef} ${conditions} ${geotag}
