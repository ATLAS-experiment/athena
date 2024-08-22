# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from PyCool import cool
import ROOT
# Work around pyroot issue with long long --- see ATEAM-997.
ROOT.gInterpreter
import os
from AthenaCommon.Logging import logging
msg = logging.getLogger( 'dummyLHCFillDB' )



def createSqlite(sqliteName,folderName="/TDAQ/OLC/LHC/FILLPARAMS"):

    dbSvc = cool.DatabaseSvcFactory.databaseService()

    if os.access(sqliteName,os.R_OK):
        msg.debug("UPDATING existing sqlite file %s" , sqliteName)
        db=dbSvc.openDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2",False)
    else:
        msg.debug("Creating new sqlite file %s" , sqliteName)
        db=dbSvc.createDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2")
        pass
       
    spec = cool.RecordSpecification()
    spec.extend("Beam1Bunches",cool.StorageType.UInt32)
    spec.extend("Beam2Bunches",cool.StorageType.UInt32)
    spec.extend("LuminousBunches",cool.StorageType.UInt32)
    spec.extend("BCIDmasks",cool.StorageType.Blob64k)

    descr='<timeStamp>time</timeStamp><addrHeader><address_header service_type="71" clid="40774348" /></addrHeader><typeName>AthenaAttributeList</typeName>'
    if db.existsFolder(folderName):
        folder=db.getFolder(folderName)
    else:
        msg.debug("Creating COOL folder/tag  %s" , (folderName))
        folderSpec = cool.FolderSpecification(cool.FolderVersioning.MULTI_VERSION, spec)
        folder = db.createFolder(folderName, folderSpec, descr, True)
        pass
      
    return db,folder



def createSqliteForAvg(sqliteName,folderName="/TDAQ/OLC/LHC/LBDATA3"):

    dbSvc = cool.DatabaseSvcFactory.databaseService()

    if os.access(sqliteName,os.R_OK):
        msg.debug("UPDATING existing sqlite file %s" , sqliteName)
        db=dbSvc.openDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2",False)
    else:
        msg.debug("Creating new sqlite file %s" , sqliteName)
        db=dbSvc.createDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2")
        pass

 

    spec = cool.RecordSpecification()
    spec.extend("Beam1Intensity",cool.StorageType.Float)
    spec.extend("Beam2Intensity",cool.StorageType.Float)

    spec.extend("Beam1IntensityAll",cool.StorageType.Float)
    spec.extend("Beam2IntensityAll",cool.StorageType.Float)
    spec.extend("Beam1IntensityStd",cool.StorageType.Float)
    spec.extend("Beam2IntensityStd",cool.StorageType.Float)
    spec.extend("Beam1IntensityAllStd",cool.StorageType.Float)
    spec.extend("Beam2IntensityAllStd",cool.StorageType.Float)

    spec.extend("RunLB",cool.StorageType.UInt63)
    spec.extend("Valid",cool.StorageType.UInt32)

    # clid=40774348 -> AthenaAttributeList
    # clid=120132775 -> BunchCrossingAverageData
    descr='<timeStamp>time</timeStamp><addrHeader><address_header service_type="71" clid="1238547719" /></addrHeader><typeName>CondAttrListCollection</typeName>'

    if db.existsFolder(folderName):
        msg.debug("folder exist")
        folder=db.getFolder(folderName)
    else:
        msg.debug("Creating COOL folder/tag  %s" , (folderName))
        folderSpec = cool.FolderSpecification(cool.FolderVersioning.MULTI_VERSION, spec)
        folder = db.createFolder(folderName, folderSpec, descr, True)
        pass
      
    return db,folder



def createSqliteForInt(sqliteName,folderName="/TDAQ/OLC/LHC/BUNCHDATA"):

    dbSvc = cool.DatabaseSvcFactory.databaseService()

    if os.access(sqliteName,os.R_OK):
        msg.debug("UPDATING existing sqlite file %s" , sqliteName)
        db=dbSvc.openDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2",False)
    else:
        msg.debug("Creating new sqlite file %s" , sqliteName)
        db=dbSvc.createDatabase("sqlite://;schema="+sqliteName+";dbname=CONDBR2")
        pass



    spec = cool.RecordSpecification()
    spec.extend("RunLB",cool.StorageType.UInt63)
    spec.extend("B1BunchAverage",cool.StorageType.Float)
    spec.extend("B2BunchAverage",cool.StorageType.Float)

    spec.extend("B1BunchIntensities",cool.StorageType.Blob64k)
    spec.extend("B2BunchIntensities",cool.StorageType.Blob64k)

    spec.extend("Valid",cool.StorageType.UInt32)
    
    # clid=40774348 -> AthenaAttributeList
    # clid=120132775 -> BunchCrossingAverageData
    descr='<timeStamp>time</timeStamp><addrHeader><address_header service_type="71" clid="1238547719" /></addrHeader><typeName>CondAttrListCollection</typeName>'

    if db.existsFolder(folderName):
        msg.debug("folder exist")
        folder=db.getFolder(folderName)
    else:
        msg.debug("Creating COOL folder/tag  %s" , (folderName))
        folderSpec = cool.FolderSpecification(cool.FolderVersioning.MULTI_VERSION, spec)
        folder = db.createFolder(folderName, folderSpec, descr, True)
        pass
      
    return db,folder


def fillFolder(folder,data=[],iovMin=cool.ValidityKeyMin,iovMax=cool.ValidityKeyMax):
    
    nB1=0
    nB2=0
    nColl=0

    for bcid in data:
        if (bcid & 0x1): nB1+=1
        if (bcid & 0x2): nB2+=1
        if (bcid & 0x3 == 0x3): nColl+=1
        pass

    
    payload=cool.Record(folder.payloadSpecification())
    payload['Beam1Bunches']=nB1
    payload['Beam2Bunches']=nB2
    payload['LuminousBunches']=nColl
    btype=getattr(ROOT,"coral::Blob")
    
    bcmask=btype()
    bcmask.resize(3564)

    for i,d in enumerate(data):
        bcmask[i]=d


    payload['BCIDmasks']=bcmask
            

    msg.debug("Storing FILLPARAMS object")
    folder.storeObject(iovMin, iovMax, payload, cool.ChannelId(0))

    return
   


def fillFolderForAvg(folder,data=[],iovMin=cool.ValidityKeyMin,iovMax=cool.ValidityKeyMax):
    
    nB1=0
    nB2=0
    nColl=0

    for bcid in data:
        if (bcid & 0x1): nB1+=1
        if (bcid & 0x2): nB2+=1
        if (bcid & 0x3 == 0x3): nColl+=1
        pass

    for chanNum in range(0,4):
        payload=cool.Record(folder.payloadSpecification())

        payload['Beam1Intensity']=1.5+1.5*chanNum
        payload['Beam2Intensity']=1.5+1.5*chanNum
        payload['Beam1IntensityAll']=1.5+1.5*chanNum
        payload['Beam2IntensityAll']=1.5+1.5*chanNum
        payload['Beam1IntensityStd']=1.5+1.5*chanNum
        payload['Beam2IntensityStd']=1.5+1.5*chanNum
        payload['Beam1IntensityAllStd']=1.5+1.5*chanNum
        payload['Beam2IntensityAllStd']=1.5+1.5*chanNum
        payload['RunLB']=430897
        payload['Valid']=1
        
                
        channelId = cool.ChannelId(chanNum)
        folder.storeObject(iovMin, iovMax, payload, channelId)
    msg.debug("Storing LBDATA3 object")


    return
   



def fillFolderForInt(folder,data=[],iovMin=cool.ValidityKeyMin,iovMax=cool.ValidityKeyMax):
    
    for chanNum in range(0,2):
        payload=cool.Record(folder.payloadSpecification())
        # payload['Channel']=0
        payload['RunLB']=430897
        payload['B1BunchAverage']=1.5+1.5*chanNum
        payload['B2BunchAverage']=1.5+1.5*chanNum


        btype=getattr(ROOT,"coral::Blob")
        
        bcmask_1=btype()
        bcmask_1.resize(3564)

        bcmask_2=btype()
        bcmask_2.resize(3564)
        
        for i,d in enumerate(data):
            bcmask_1[i]=d

        for i,d in enumerate(data):
            bcmask_2[i]=d

        payload['B1BunchIntensities']=bcmask_1
        payload['B2BunchIntensities']=bcmask_2
        payload['Valid']=1
        
        channelId = cool.ChannelId(chanNum)
        folder.storeObject(iovMin, iovMax, payload, channelId)

    msg.debug("Storing BUNCHDATA object")

    return
   


def createBCMask1():
    mask=[]
    #pre-fill with zero
    for i in range(0,3564):
        mask.append(0x0)    

    #A train across the wrap-around:
    for i in range (0,25):
        mask[i]=0x3
    
    for i in range (3550,3564):
        mask[i]=0x3

    #A short sequence of bunches that doesn't qualify as train
    for i in range (1000,1030):

        mask[i]=0x3
    return mask
         

def createBCMask2():
    mask=[]
    #pre-fill with zero
    for i in range(0,3564):
        mask.append(0x0)    

    t8b4e=[0x3,0x3,0x3,0x3, 0x3,0x3,0x3,0x3, 0x0,0x0,0x0,0x0]
    
    for i in range(0,20):
        #create a train of 20 8be4 groups start
        mask[100+i*12:100+(i+1)*12]=t8b4e

    return mask




if __name__=="__main__":


    db,folder=createSqlite("test.db")
    
    d1=createBCMask1()
    d2=createBCMask2()
    
    fillFolder(folder,d1,iovMin=cool.ValidityKey(1e6),iovMax=cool.ValidityKey(2e6))
    fillFolder(folder,d2,iovMin=cool.ValidityKey(2e6),iovMax=cool.ValidityKey(3e6))


    db.closeDatabase()
    
   
