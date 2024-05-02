# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloCellNoiseAlgCfg(flags):
    result=ComponentAccumulator()
    from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
    result.merge(LArADC2MeVCondAlgCfg(flags))

    noiseAlg=CompFactory.CaloCellNoiseAlg("CaloCellNoiseAlg",
                                          doMC = True,
                                          readNtuple = False,
                                          doFit = True,
                                          TotalNoiseKey = '',
                                          ElecNoiseKey = '',
                                         )
    result.addEventAlgo(noiseAlg)

    result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='cellnoise.root' OPT='NEW'" ]))
    result.setAppProperty("HistogramPersistency","ROOT")
    return result


    
                                      
if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.isMC=True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True),sequenceName="AthAlgSeq")

    from CaloRec.CaloCellMakerConfig import CaloCellMakerCfg
    cfg.merge(CaloCellMakerCfg(flags))
    cfg.merge(CaloCellNoiseAlgCfg(flags))
              
    cfg.run(10)

    
