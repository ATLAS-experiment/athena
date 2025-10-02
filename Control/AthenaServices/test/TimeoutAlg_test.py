#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Testing job for TimeoutAlg
#
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaCommon.Constants import DEBUG
from AthenaCommon.SystemOfUnits import millisecond
from AthenaPython.PyAthena import Alg, StatusCode
import sys
import time

burnTime = 150 # ms

class CpuBurner(Alg):
   def execute(self):
      time.sleep(burnTime/1000)
      return StatusCode.Success

flags = initConfigFlags()
flags.Exec.MaxEvents = 4
flags.Concurrency.NumThreads = 2
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)
cfg.addEventAlgo(CompFactory.TimeoutAlg(Timeout = 50*millisecond,
                                        AbortJob = False,
                                        DumpSchedulerState = False,
                                        OutputLevel = DEBUG))

try:
   cpuBurner = CompFactory.PerfMonTest.CpuCruncherAlg(MeanCpu = burnTime)
except AttributeError:
   # For non-Athena projects, use Python alg for testing
   cpuBurner = CpuBurner()

cfg.addEventAlgo(cpuBurner)

sys.exit(not cfg.run().isSuccess())
