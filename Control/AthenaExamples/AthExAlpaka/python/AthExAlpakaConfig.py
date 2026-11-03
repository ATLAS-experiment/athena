#!/usr/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Simple ComponentAccumulator configuration for running
# AlpakaExampleAlg

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

import sys

# Set up the job's flags.
flags = initConfigFlags()
flags.Exec.MaxEvents = 10
flags.Input.Files = []
flags.fillFromArgs()
flags.lock()

# Set up the main services.
acc = MainServicesCfg(flags)

# Set up the example algorithm.
acc.addEventAlgo(CompFactory.AlpakaExampleAlg())

# Run the configuration.
sys.exit(acc.run().isFailure())

