#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
from GlobalSimulation.LArCellPreparationAlgConfig import LArCellPreparationAlgCfg
from GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
from PathResolver import PathResolver
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig

def GlobalSimulationCfg(flags, algLogLevel=None):

    cfg = ComponentAccumulator()

    if algLogLevel is None:
        algLogLevel = flags.Exec.OutputLevel

    cfg.merge(LArCellPreparationAlgCfg(
        flags,
        numberOfEnergyBits=6,
        valueLeastSignificantBit=40,
        valueGainFactor=4,
        GlobalLArCellsKey="GlobalLArCells",
        caloCells="SeedLessFS",
        OutputLevel=algLogLevel
    ))

    cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags, OutputLevel=algLogLevel))

    cfg_fn = PathResolver.FindCalibFile("GlobalSimulation/globalSim_AllChainsCfg.xml")
    cfg.merge(GlobalSimulationAlgCfg(flags, fn=cfg_fn, dump=False, OutputLevel=algLogLevel))

    cfg.merge(TriggerHistSvcConfig(flags))

    return cfg
