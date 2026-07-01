# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# run this with: l1calo-ath-mon --postInclude GlobalSimulation/plugin_hypo_mult.py --evtMax 10 --filesInput RAW_RUN3_DATA24
#cfg and flags are already defined

# add in the Algorithm to be run
from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
from PathResolver import PathResolver
cfg_fn = PathResolver.FindCalibFile("GlobalSimulation/globalSim_hypo_mult.xml") # should probably check file is found!
cfg.merge(GlobalSimulationAlgCfg(flags, fn=cfg_fn, dump=True))
