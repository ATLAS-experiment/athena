# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from LumiBlockComps.BunchCrossingAverageCondAlgConfig import BunchCrossingAverageCondAlgCfg
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
from LumiBlockComps.dummyLHCFillDB import createSqliteForAvg,fillFolderForAvg,createBCMask1,createBCMask2
import os



#First, create a dummy database to work with:
#Delete any previous instance, if there is any:
try:
    os.remove("testAVG.db")
except OSError:
    pass


db,folder=createSqliteForAvg("testAVG.db",folderName="/TDAQ/OLC/LHC/LBDATA3")
d1=createBCMask1()
d2=createBCMask2()

onesec=1000000000
#Add two dummy masks with iov 2-3 and 3-4
fillFolderForAvg(folder,d1,iovMin=1*onesec,iovMax=2*onesec)
fillFolderForAvg(folder,d2,2*onesec,4*onesec)

db.closeDatabase()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
flags = initConfigFlags()
flags.Input.Files=[]
flags.Input.isMC=False
flags.IOVDb.DatabaseInstance="CONDBR2"
flags.IOVDb.GlobalTag="CONDBR2-BLKPA-2017-05"
from AthenaConfiguration.TestDefaults import defaultGeometryTags
flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
flags.lock()
result=MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
#The time stamp correspond to: Wednesday 10 August 2024 22:26:00 (Run 430897)
result.merge(McEventSelectorCfg(flags,
                                RunNumber=430897,
                                EventsPerRun=1,
                                FirstEvent=1183722158,
                                FirstLB=310,
                                EventsPerLB=1,
                                InitialTimeStamp=1,
                                TimeStampInterval=1))

result.merge(BunchCrossingAverageCondAlgCfg(flags))
result.merge(IOVDbSvcCfg(flags))
result.getService("IOVDbSvc").Folders=["<db>sqlite://;schema=testAVG.db;dbname=CONDBR2</db><tag>HEAD</tag>/TDAQ/OLC/LHC/LBDATA3"]
result.getCondAlgo("BunchCrossingAverageCondAlgDefault").OutputLevel=1
BunchCrossingAverageCondTest=CompFactory.BunchCrossingAverageCondTest
result.addEventAlgo(BunchCrossingAverageCondTest(FileName="BCAvgData.txt"))
result.run(1)
