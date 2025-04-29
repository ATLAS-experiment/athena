#!/usr/bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Simple ComponentAccumulator configuration for running
# AthCUDAExamples::LinearTransformAsyncExampleAlg#

# Core import(s).
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles

# I/O import(s).
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

# System import(s).
import sys

def LinearTransformAsyncExampleAlgCfg(flags, **kwargs):
   '''Configure the example algorithm for running on a CUDA device.
   '''
   # Create an accumulator to hold the configuration.
   result = ComponentAccumulator()
   # Create the example algorithm.
   alg = CompFactory.AthCUDAExamples.LinearTransformAsyncExampleAlg(**kwargs)
   result.addEventAlgo(alg)
   # Return the result to the caller.
   return result

if __name__ == '__main__':

   # Set up the job's flags.
   flags = initConfigFlags()
   flags.Concurrency.NumOffloadThreads = 2
   flags.Exec.MaxEvents = 10000
   flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
   flags.fillFromArgs()
   flags.lock()

   # Set up the main services.
   acc = MainServicesCfg(flags)

   # Set up the input file reading.
   acc.merge(PoolReadCfg(flags))

   # Set up the example algorithm.
   acc.merge(LinearTransformAsyncExampleAlgCfg(flags))

   # Run the configuration.
   sys.exit(acc.run().isFailure())
