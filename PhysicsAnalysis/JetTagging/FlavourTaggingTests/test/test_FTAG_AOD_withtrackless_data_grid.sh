#!/bin/sh
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# art-description: RDO to AOD step with trackless b-tagging on data 2023
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: *.pool.root
# art-output: *.log
# art-output: *log.
# art-athena-mt: 8

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_DATA22)")

ATHENA_CORE_NUMBER=4 Reco_tf.py \
--multithreaded \
--AMIConfig q449 \
--conditionsTag $conditions \
--imf False \
--preExec="all:flags.BTagging.Trackless=True" \
--maxEvents 25

echo "art-result: $? AOD_Creation"

