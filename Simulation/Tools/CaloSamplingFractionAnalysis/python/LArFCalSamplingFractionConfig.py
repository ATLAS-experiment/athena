#!/usr/bin/env athena.py

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This CA configuration script replaces the previously used legacy script for simulating sampling fractions
#

import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
from LArGeoAlgsNV.LArGMConfig import LArGMCfg

# Adding algorithm
def LarFCalSamplingFractionCfg(flags, name="LarFCalSamplingFraction", **kwargs):
    acc = ComponentAccumulator()

    acc.addService(CompFactory.THistSvc(name="THistSvc", Output=[ f"AANT DATAFILE='{flags.Output.HISTFileName}' OPT='RECREATE'" ]))

    kwargs.setdefault('Calibration', True)

    acc.merge( TileCablingSvcCfg(flags) )
    acc.merge( TileSamplingFractionCondAlgCfg(flags) )
    acc.getCondAlgo('TileSamplingFractionCondAlg').G4Version=-1
    requiredConditons=["Shape","AutoCorr","Noise","Pedestal","fSampl","MinBias"]
    acc.merge(LArElecCalibDBCfg(flags,requiredConditons))
    acc.merge(LArGMCfg(flags))
    acc.addEventAlgo(CompFactory.LArFCalSamplingFraction(name, **kwargs))

    return acc

if __name__ == "__main__":
  # Setting flags
  flags = initConfigFlags()
  flags.IOVDb.GlobalTag = 'OFLCOND-MC23-SDR-RUN3-04'
  flags.Input.Files = ['test.root']
  flags.Output.HISTFileName = 'LArFCal_SF.root'
  flags.Exec.MaxEvents = -1
  flags.fillFromArgs()
  flags.dump()
  flags.lock()

  # Main CA and basic services
  acc = MainServicesCfg(flags)
  acc.merge(PoolReadCfg(flags))
  acc.merge(LarFCalSamplingFractionCfg(flags))
  acc.printConfig(withDetails=True)

  # Finalize
  sys.exit( acc.run().isFailure() )
