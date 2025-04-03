#!/bin/sh
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# art-description: RDO to AOD step with trackless b-tagging for Run 3 MC 
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: *.pool.root
# art-output: *.log
# art-output: *log.
# art-athena-mt: 8

ATHENA_CORE_NUMBER=4

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

Reco_tf.py \
--multithreaded \
--AMIConfig q454 \
--conditionsTag "default:${conditions}" \
--steering doRAWtoALL \
--imf False \
--CA all:True \
--preExec="all:flags.BTagging.Trackless=True" \
--maxEvents 25

echo "art-result: $? AOD_Creation"

