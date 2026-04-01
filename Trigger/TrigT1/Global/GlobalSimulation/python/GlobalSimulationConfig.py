#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# placeholder file for GlobalSimulation cfg fragment

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GlobalSimulationCfg(flags):
    cfg = ComponentAccumulator()

    # todo: A GlobalSimulation configured by flags and any extra arguments
    from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
    from PathResolver import PathResolver
    cfg_fn = PathResolver.FindCalibFile("GlobalSimulation/globalSim_hypo_mult.xml")
    cfg.merge(GlobalSimulationAlgCfg(flags, fn=cfg_fn, dump=True))

    return cfg
