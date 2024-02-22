#/bin/bash
#
# Copyright (C) 2002-2022  CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


set -e
set -u

dataType=$1
shift
athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType --for-compare --block-config "$@"
athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType --for-compare "$@"


acmd.py diff-root --error-mode resilient -t analysis FullCPAlgorithmsTest.$dataType.hist.root FullCPAlgorithmsConfigTest.$dataType.hist.root
