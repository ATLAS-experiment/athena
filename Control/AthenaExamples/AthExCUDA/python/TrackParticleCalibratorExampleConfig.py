#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Simple ComponentAccumulator configuration for running
# AthCUDAExamples::TrackParticleCalibratorExampleAlg, offloading trivial
# operations on xAOD::TrackParticleContrinaer, to a CUDA device.
#

# Core import(s).
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaCommon.Constants import DEBUG

# I/O import(s).
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

# Device/Accelerator import(s).
from AthCUDAServices.AthCUDAServicesConfig import \
   HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg

# System import(s).
import sys

def TrackParticleCalibratorExampleAlgCfg(flags, **kwargs):
   '''Configure the example algorithm for running on a CUDA device.
   '''
   # Create an accumulator to hold the configuration.
   result = ComponentAccumulator()
   # Create the example algorithm.
   alg = CompFactory.AthCUDAExamples.TrackParticleCalibratorExampleAlg(**kwargs)
   hostMR = HostMemoryResourceToolCfg(flags, **kwargs)
   alg.HostMR = hostMR.getPrimary()
   result.merge(hostMR)
   deviceMR = DeviceMemoryResourceToolCfg(flags, **kwargs)
   alg.DeviceMR = deviceMR.getPrimary()
   result.merge(deviceMR)
   # Add the algorithm to the accumulator, so that it would eventually be
   # scheduled to run.
   result.addEventAlgo(alg)
   # Return the result to the caller.
   return result

if __name__ == '__main__':

   # Set up the job's flags.
   flags = initConfigFlags()
   flags.Exec.MaxEvents = 100
   flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
   flags.fillFromArgs()
   flags.lock()

   # Set up the main services.
   acc = MainServicesCfg(flags)

   # Set up the input file reading.
   acc.merge(PoolReadCfg(flags))

   # Set up the example algorithm.
   acc.merge(TrackParticleCalibratorExampleAlgCfg(flags, OutputLevel = DEBUG))

   # Run the configuration.
   sys.exit(acc.run().isFailure())
