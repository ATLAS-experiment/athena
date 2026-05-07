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

# add in the Algorithm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=False,
                                     dumpTerse=False))

cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etMin = 5000.
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMin = 0.0
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMax = 5.0

alg = CompFactory.GlobalSim.GlobalSimulationAlg('GlobalSimulationAlg')

# add in the EgammaBDT Algorithm to be run
bdtTool =  CompFactory.GlobalSim.Egamma1BDTAlgTool('Egamma1BDTAlgTool')
bdtTool.enableDump = False
bdtTool.OutputLevel = DEBUG

from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
cfg.merge(addToAOD(flags,["std::vector<float>#eGamma1BDT"]))
cfg.merge(addToAOD(flags,["std::vector<float>#eFEXeta"]))
cfg.merge(addToAOD(flags,["std::vector<float>#eFEXphi"]))
cfg.merge(addToAOD(flags,["std::vector<float>#FailedeFEXeta"]))
cfg.merge(addToAOD(flags,["std::vector<float>#FailedeFEXphi"]))

alg.globalsim_algs = [bdtTool]

# add in the EgammaBDTMultiplicity Algorithm to be run
tool =  CompFactory.GlobalSim.eEmEg1BDTMultAlgTool('eEmEg1BDTMultAlgTool')
tool.Eg1BDT = '0'
tool.Eg1BDT_op = '>='
tool.enable_dump=True
tool.OutputLevel = DEBUG

alg.TIPwriters = [tool]

cfg.addEventAlgo(alg)
