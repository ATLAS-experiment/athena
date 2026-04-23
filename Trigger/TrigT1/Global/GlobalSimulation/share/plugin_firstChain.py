from AthenaCommon.Constants import DEBUG

# Add algorithms to run tower building from GlobalLArCells

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

# Algorithm to build cell towers                                                                                                                                                             
from  GlobalSimulation.GlobalCellTowerAlgToolConfig import GlobalCellTowerAlgToolCfg
cfg.merge(GlobalCellTowerAlgToolCfg(flags,
                                    GlobalLArCellsKey = gblLArCellContainerKey,
                                    GlobalCellTowersKey = "GlobalCellTowers",
                                    OutputLevel=DEBUG))
