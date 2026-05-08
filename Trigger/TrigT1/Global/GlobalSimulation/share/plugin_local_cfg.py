# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# run this with: l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py --evtMax 10 --filesInput RAW_RUN3_DATA24
#cfg and flags are already defined

from AthenaCommon.Constants import DEBUG, INFO

# Add algorithm to prepare LAr cells for Global                                                                                                                                                                                                                           
from  GlobalSimulation.LArCellPreparationAlgConfig import LArCellPreparationAlgCfg
gblLArCellContainerKey = "GlobalLArCells"
cfg.merge(LArCellPreparationAlgCfg(flags,
                                   numberOfEnergyBits = 6,
                                   valueLeastSignificantBit = 40,
                                   valueGainFactor = 4,
                                   GlobalLArCellsKey = gblLArCellContainerKey,
                                   OutputLevel=DEBUG))

# add in the Algortihm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                     OutputLevel=INFO,
                                     dump=True,
                                     dumpTerse=False))

from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
cfg.merge(GlobalSimulationAlgCfg(flags, dump=True, OutputLevel=DEBUG))

