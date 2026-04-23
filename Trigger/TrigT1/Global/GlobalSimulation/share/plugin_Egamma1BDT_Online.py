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

# add in the Algortihm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_OnlineMapNbhoodConfig import Egamma1_OnlineMapNbhoodCfg
cfg.merge(Egamma1_OnlineMapNbhoodCfg(flags,
                                     OutputLevel=DEBUG,
                                     dump=True,
                                     dumpTerse=False))

cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etMin = 5000.
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMin = 0.0
cfg.getEventAlgo("Egamma1_OnlineMapNbhood").roiAlgTool.etaMax = 5.0

# add in the EgammaBDT Algorithm to be run
from GlobalSimulation.GlobalSimAlgCfg_Egamma1BDT  import GlobalSimulationAlgCfg
cfg.merge(GlobalSimulationAlgCfg(flags,
                                 OutputLevel=DEBUG,
                                 dump=False))

from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
cfg.merge(addToAOD(flags,["std::vector<float>#eGamma1BDT"]))
cfg.merge(addToAOD(flags,["std::vector<float>#eFEXeta"]))
cfg.merge(addToAOD(flags,["std::vector<float>#eFEXphi"]))
cfg.merge(addToAOD(flags,["std::vector<float>#FailedeFEXeta"]))
cfg.merge(addToAOD(flags,["std::vector<float>#FailedeFEXphi"]))
