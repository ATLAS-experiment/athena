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

# Add algorithm to simulate MUX input/output for LAr cells
from  GlobalSimulation.LArCellMuxAlgConfig import LArCellMuxAlgCfg
cfg.merge(LArCellMuxAlgCfg(flags,
                           GlobalLArCellsKey = gblLArCellContainerKey,
                           WriteMuxInputBitstreamToFile = True,
                           WriteMuxOutputBitstreamToFile = True,
                           OutputLevel=DEBUG))

# add in the Algorithm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=False,
                                     dumpTerse=False))

cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etMin = 5000.
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMin = 0.0
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMax = 5.0

from GlobalSimulation.GlobalSimAlgCfg_local import GlobalSimulationAlgCfg
cfg.merge(GlobalSimulationAlgCfg(flags, dump=True, OutputLevel=DEBUG))

