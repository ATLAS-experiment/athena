#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
Test to run PMGTruthWeightsAlg
"""

_input_help = "with no inputs use ASG_TEST_FILE_MC"

from argparse import ArgumentParser
import os

from GaudiKernel.Configurable import DEBUG
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AsgAnalysisAlgorithms.PMGTruthWeightAlgConfig import PMGTruthWeightAlgCfg

def get_parser():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument('input_files', nargs='*', help=_input_help)
    parser.add_argument('-m', '--max-events', type=int, const=10, nargs='?')
    parser.add_argument('-d', '--debug-logs', action='store_true')
    return parser.parse_args()

def run():
    args = get_parser()
    flags = initConfigFlags()
    flags.Input.Files = args.input_files or [os.environ['ASG_TEST_FILE_MC']]
    if args.debug_logs:
        flags.Exec.OutputLevel = DEBUG
    if args.max_events:
        flags.Exec.MaxEvents = args.max_events
    ca = MainServicesCfg(flags)
    ca.merge(PoolReadCfg(flags))

    ca.merge(PMGTruthWeightAlgCfg(flags))

    ca.run()

if __name__ == '__main__':
    run()
