# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.Constants import DEBUG

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

# Add the GlobalSim Algorithm
alg = CompFactory.GlobalSim.GlobalSimulationAlg('GlobalSimulationAlg')

# Attach the CellTower AlgTool
cellTowerTool =  CompFactory.GlobalSim.GlobalCellTowerAlgTool('GlobalCellTowerAlgTool')
cellTowerTool.GlobalCellTowersKey = "GlobalCellTowers"
cellTowerTool.OutputLevel = DEBUG

alg.globalsim_algs += [cellTowerTool]

# Attach the WTAConeJet AlgTool
jet1Tool =  CompFactory.GlobalSim.GlobalJet1AlgTool('GlobalJet1AlgTool')
jet1Tool.GlobalCellTowersKey = "GlobalCellTowers"
jet1Tool.OutputLevel = DEBUG

alg.globalsim_algs += [jet1Tool]

# add in the JetMultiplicity AlgTool
tool =  CompFactory.GlobalSim.JetMultAlgTool('JetMultAlgTool')

tool.et_low = '2500'
tool.enable_dump=True
tool.OutputLevel = DEBUG

alg.TIPwriters = [tool]

cfg.addEventAlgo(alg)
