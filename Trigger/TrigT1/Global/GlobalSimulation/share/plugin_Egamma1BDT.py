from AthenaCommon.Constants import DEBUG

# add in the Algortihm to build a  LArStrip Neighborhood container
from  GlobalSimulation.Egamma1_LArStrip_Fex_RowAwareConfig import Egamma1_LArStrip_Fex_RowAwareCfg
cfg.merge(Egamma1_LArStrip_Fex_RowAwareCfg(flags,
                                           OutputLevel=DEBUG,
                                           dump=False,
                                           dumpTerse=False))

cfg.getEventAlgo("Egamma1_LArStrip_Fex_RowAware").caloCellProducer.makeCaloCellContainerChecks = False
cfg.getEventAlgo("Egamma1_LArStrip_Fex_RowAware").roiAlgTool.etMin = 5000.
cfg.getEventAlgo("Egamma1_LArStrip_Fex_RowAware").roiAlgTool.etaMin = 0.2
cfg.getEventAlgo("Egamma1_LArStrip_Fex_RowAware").roiAlgTool.etaMax = 1.4

# add in the EgammaBDT Algorithm to be run
from GlobalSimulation.GlobalSimAlgCfg_Egamma1BDT  import GlobalSimulationAlgCfg
cfg.merge(GlobalSimulationAlgCfg(flags,
                                 OutputLevel=DEBUG,
                                 dump=False))

from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
cfg.merge(addToAOD(flags,["std::vector<float>#eGamma1BDT"]))
