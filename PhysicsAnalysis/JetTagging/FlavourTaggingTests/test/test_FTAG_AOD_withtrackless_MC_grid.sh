#!/bin/sh
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# art-description: RDO to AOD step with trackless b-tagging for Run 3 MC 
# art-type: grid
# art-include: main/Athena
# art-output: *.pool.root
# art-output: *.log
# art-output: *log.
# art-athena-mt: 4

ATHENA_CORE_NUMBER=4 Reco_tf.py \
--multithreaded \
--AMIConfig q445 \
--imf False \
--CA all:True RDOtoRDOTrigger:False \
--preExec="all:flags.BTagging.Trackless=True" \
--maxEvents 25

echo "art-result: $? AOD_Creation"

