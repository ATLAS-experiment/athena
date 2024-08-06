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


# Adding algorithm
def LarEMSamplingFractionCfg(flags, name="LarEMSamplingFraction", **kwargs):
    acc = ComponentAccumulator()

    acc.addService(CompFactory.THistSvc(name="THistSvc", Output=[ f"{name} DATAFILE='{flags.Output.HISTFileName}' OPT='RECREATE'" ]))

    kwargs.setdefault('HistPath', f"/{name}/")
    kwargs.setdefault('DoCells', 0)

    kwargs.setdefault('CalibrationHitContainerNames', [
        "LArCalibrationHitInactive",
        "LArCalibrationHitActive",
        "TileCalibHitInactiveCell",
        "TileCalibHitActiveCell",
    ] )

    acc.addEventAlgo(CompFactory.LarEMSamplingFraction(name, **kwargs))

    return acc


# Setting flags
flags = initConfigFlags()
flags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'
flags.Input.Files = ['test.root']
flags.Output.HISTFileName = 'LArEM_SF.root'
flags.Exec.MaxEvents = -1
flags.fillFromArgs()
flags.dump()
flags.lock()

# Main CA and basic services
acc = MainServicesCfg(flags)
acc.merge(LarEMSamplingFractionCfg(flags))
acc.merge(PoolReadCfg(flags))

# Finalize
acc.run()

sys.exit( acc.run().isFailure() )
