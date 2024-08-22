# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
from LumiBlockComps.dummyLHCFillDB import createSqliteForInt,fillFolderForInt,createBCMask1,createBCMask2



def BunchCrossingIntensityCondAlgCfg(configFlags):

    result=ComponentAccumulator()
    run1=(configFlags.IOVDb.DatabaseInstance=='COMP200')
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    Mode = 0
    folder = '/TDAQ/OLC/LHC/BUNCHDATA'
    result.merge(addFolders(configFlags,folder,'TDAQ',className = 'CondAttrListCollection',tag='HEAD'))
    alg = CompFactory.BunchCrossingIntensityCondAlg('BunchCrossingIntensityCondAlgDefault',
                               Run1=run1,
                               FillParamsFolderKey =folder,
                               Mode=Mode )

    result.addCondAlgo(alg)

    return result



if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC=False
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.IOVDb.GlobalTag="CONDBR2-BLKPA-2017-05"
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result=MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg




    db,folder=createSqliteForInt("testInt.db",folderName="/TDAQ/OLC/LHC/BUNCHDATA")
    d1=createBCMask1()
    d2=createBCMask2()

    onesec=1000000000


    #Add two dummy masks with iov 2-3 and 3-4
    fillFolderForInt(folder,d1,iovMin=1*onesec,iovMax=2*onesec)
    fillFolderForInt(folder,d2,2*onesec,4*onesec)

    db.closeDatabase()
    #The time stamp correspond to: Wednesday 10 August 2024 22:26:00 (Run 430897)
    result.merge(McEventSelectorCfg(flags,
                                RunNumber=430897,
                                EventsPerRun=1,
                                FirstEvent=1183722158,
                                FirstLB=310,
                                EventsPerLB=1,
                                InitialTimeStamp=1,
                                TimeStampInterval=1))



    result.merge(BunchCrossingIntensityCondAlgCfg(flags))
    result.merge(IOVDbSvcCfg(flags))
    result.getService("IOVDbSvc").Folders=["<db>sqlite://;schema=testInt.db;dbname=CONDBR2</db><tag>HEAD</tag>/TDAQ/OLC/LHC/BUNCHDATA"]
    BunchCrossingIntensityCondTest=CompFactory.BunchCrossingIntensityCondTest
    result.addEventAlgo(BunchCrossingIntensityCondTest(FileName="BCIntData1.txt"))

    result.run(1)