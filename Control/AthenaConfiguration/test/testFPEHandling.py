#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from sys import argv  

if __name__=="__main__":
  
  FPEFlag=0
  if len(argv)>1:
    FPEFlag=int(argv[1])

  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()
  flags.Input.Files = []
  flags.Input.RunNumbers = [284500] # dummay value
  flags.Input.TimeStamps = [1] # dummy value
  flags.Exec.FPE=FPEFlag
  cfg = MainEvgenServicesCfg(flags)
  
  cfg.addEventAlgo(CompFactory.AthExAlgWithFPE(),sequenceName="AthAlgSeq")
  
  cfg.run(3)
