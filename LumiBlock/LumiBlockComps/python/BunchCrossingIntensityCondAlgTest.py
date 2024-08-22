# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from LumiBlockComps.BunchCrossingIntensityCondAlgConfig import BunchCrossingIntensityCondAlgCfg
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
from LumiBlockComps.dummyLHCFillDB import createSqliteForInt,fillFolderForInt,createBCMask1,createBCMask2
import os



#First, create a dummy database to work with:
#Delete any previous instance, if there is any:
try:
    os.remove("testInt.db")
except OSError:
    pass


db,folder=createSqliteForInt("testInt.db",folderName="/TDAQ/OLC/LHC/BUNCHDATA")
d1=createBCMask1()
d2=createBCMask2()

onesec=1000000000

#Add two dummy masks with iov 2-3 and 3-4
fillFolderForInt(folder,d1,iovMin=1*onesec,iovMax=2*onesec)
fillFolderForInt(folder,d2,2*onesec,4*onesec)
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
result.getCondAlgo("BunchCrossingIntensityCondAlgDefault").OutputLevel=1

BunchCrossingIntensityCondTest=CompFactory.BunchCrossingIntensityCondTest
result.addEventAlgo(BunchCrossingIntensityCondTest(FileName="BCIntData.txt"))


result.run(1)
