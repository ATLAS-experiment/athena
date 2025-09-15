# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from Campaigns.Utils import Campaign
import sys
from pathlib import Path

def LArConditionsTestCfg(flags):
    #Get basic services and cond-algos
    result=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    result.merge(LArCalibIdMappingCfg(flags))

    
    LArConditionsTestAlg=CompFactory.LArConditionsTestAlg()
    LArConditionsTestAlg.OutputLevel    = 2
    LArConditionsTestAlg.TestCondObjs     = True
    LArConditionsTestAlg.ApplyCorrections = True
    if (flags.LArCondTest.Step==2):
        #Reading case:
        LArConditionsTestAlg.ReadCondObjs     = True
        LArConditionsTestAlg.WriteCondObjs    = False
        LArConditionsTestAlg.WriteCorrections = False 
    else:
        #Writing case:
        LArConditionsTestAlg.ReadCondObjs     = False
        LArConditionsTestAlg.WriteCondObjs    = True
        LArConditionsTestAlg.WriteCorrections = True     
    result.addEventAlgo(LArConditionsTestAlg)

    if (flags.LArCondTest.Step==1):
        #Write to pool.root file
        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        result.merge(OutputConditionsAlgCfg(flags,
                                            outputFile="LarCondTestNoReg.root",
                                            ObjectList=["LArRampMC#/LArCalorimeter/LArTests/LArRampsSingleGroup",
                                                        "LArRampMC#/LArCalorimeter/LArTests/LArRampsSubDetectorGrouping",
                                                        "LArRampMC#/LArCalorimeter/LArTests/LArRampsFeedThroughGrouping" ],
                                            IOVTagList=["mytag"],
                                            Run1=3,
                                            Run2=9999,
                                            WriteIOV=False
                                            ))
    elif (flags.LArCondTest.Step==2):
        #Read the pool-root file
        from EventSelectorAthenaPool.CondProxyProviderConfig import CondProxyProviderCfg 
        result.merge(CondProxyProviderCfg(flags,"LarCondTestNoReg.root"))


    
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags=initConfigFlags()
    flags.addFlag("LArCondTest.Step",1)
    #Set a few flags to avoid input-file peeking (there is no input)
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.Input.isMC=True
    flags.IOVDb.GlobalTag="OFLCOND-CSC-00-01-00"
    flags.Input.Files=[]
    flags.Input.TypedCollections=[]
    flags.Input.MCCampaign=Campaign.Unknown
    flags.fillFromArgs()
    flags.lock()
    cfg=MainServicesCfg(flags)
    cfg.merge(LArConditionsTestCfg(flags))

    msgsvc=cfg.getService("MessageSvc")
    msgsvc.debugLimit=100000
    #Force old msg format so that output comparison succeeds
    msgsvc.Format='% F%18W%S%7W%R%T %0W%M'
    
    poolSvc=cfg.getService("PoolSvc")
    poolSvc.WriteCatalog=f"file:Catalog{flags.LArCondTest.Step}.xml"

    # remove catalog on re-run to help with incremental testing
    catalogFile = Path(f"Catalog{flags.LArCondTest.Step}.xml")
    if catalogFile.exists():
        catalogFile.unlink()

    sys.exit(cfg.run(1).isFailure())
