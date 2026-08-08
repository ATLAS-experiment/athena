# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
import os
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.Logging import logging
from functools import cache

msg = logging.getLogger('IOVDbSvcCfg')

def CondInputLoaderCfg(flags, **kwargs):
    result = ComponentAccumulator()
    result.addCondAlgo(CompFactory.CondInputLoader(**kwargs))
    return result


def DBReplicaSvcCfg(flags, vetoDBRelease=False, **kwargs):
    if vetoDBRelease:
        kwargs.setdefault('COOLSQLiteVetoPattern', '/DBRelease/')

    result = ComponentAccumulator()
    result.addService(CompFactory.DBReplicaSvc(**kwargs))
    return result


@AccumulatorCache
def IOVDbSvcCfg(flags, **kwargs):
    # Add the conditions loader, must be the first in the sequence
    result = CondInputLoaderCfg(flags)

    kwargs.setdefault('OnlineMode', flags.Common.isOnline)
    kwargs.setdefault('dbConnection', flags.IOVDb.DBConnection)
    # setup knowledge of dbinstance in IOVDbSvc, for global tag x-check
    kwargs.setdefault('DBInstance', flags.IOVDb.DatabaseInstance)

    if 'FRONTIER_SERVER' in os.environ.keys() and os.environ['FRONTIER_SERVER'] != '':
        kwargs.setdefault('CacheAlign', 3)

    # Very important cache settings for use of CoralProxy at P1 (ATR-4646)
    if flags.Common.isOnline and flags.Trigger.Online.isPartition:
        kwargs['CacheAlign'] = 0
        kwargs['CacheRun'] = 0
        kwargs['CacheTime'] = 0

    kwargs.setdefault('GlobalTag', flags.IOVDb.GlobalTag)
    if 'Folders' in kwargs:
        kwargs['Folders'] = ['/TagInfo<metaOnly/>'] + kwargs['Folders']
    else:
        kwargs.setdefault('Folders', ['/TagInfo<metaOnly/>'])

    # Select CREST backend if needed
    if flags.IOVDb.UseCREST:
        kwargs.setdefault('Source', 'CREST')
        checkGlobalTag(flags.IOVDb.DBConnection,flags.IOVDb.GlobalTag)

    result.addService(CompFactory.IOVDbSvc(**kwargs), primary=True)

    # Set up POOLSvc with appropriate catalogs
    from AthenaPoolCnvSvc.PoolCommonConfig import PoolSvcCfg
    result.merge(PoolSvcCfg(flags, withCatalogs=True))
    if flags.MP.UseSharedReader or flags.MP.UseSharedWriter:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolSharedIOCnvSvcCfg
        result.merge(AthenaPoolSharedIOCnvSvcCfg(flags))
    else:
        from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
        result.merge(AthenaPoolCnvSvcCfg(flags))
    result.addService(CompFactory.CondSvc())
    result.addService(CompFactory.ProxyProviderSvc(ProviderNames=['IOVDbSvc']))

    if not flags.Input.isMC and not flags.IOVDb.UseCREST:
        result.merge(DBReplicaSvcCfg(flags, vetoDBRelease=True))

    # Get TagInfoMgr
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    result.merge(TagInfoMgrCfg(flags))

    # Set up MetaDataSvc
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    result.merge(MetaDataSvcCfg(flags, ['IOVDbMetaDataTool']))

    return result


# Convenience method to add folders:
def addFolders(flags, folderStrings, detDb=None, className=None, extensible=False, tag=None, db=None, modifiers=''):
    tagString = ''
    if tag is not None:
        if flags.IOVDb.UseCREST:
            tagString = '<ctag>%s</ctag>' % tag
        else: #COOL variant
            tagString = '<tag>%s</tag>' % tag

    # Convenience hack: Allow a single string as parameter:
    if isinstance(folderStrings, str):
        return addFolderList(flags, ((folderStrings + tagString, detDb, className),), extensible, db, modifiers)

    else: # Got a list of folders
        folderDefinitions = []

        for folderString in folderStrings:
            folderDefinitions.append((folderString + tagString, detDb, className))

    return addFolderList(flags, folderDefinitions, extensible, db, modifiers)


def addFolderList(flags, listOfFolderInfoTuple, extensible=False, db=None, modifiers=''):
    """Add access to the given set of folders, in the identified subdetector schema.
    FolerInfoTuple consists of (foldername,detDB,classname)

    If EXTENSIBLE is set, then if we access an open-ended IOV at the end of the list,
    the end time for this range will be set to just past the current event.
    Subsequent accesses will update this end time for subsequent events.
    This allows the possibility of later adding a new IOV using IOVSvc::setRange."""
    loadFolders = set()
    folders = []
    if flags.IOVDb.UseCREST:
        sqliteFolders=getCrestDirContent(flags)
    else:
        sqliteFolders=getSqliteContent(flags.IOVDb.SqliteInput,
                                       flags.IOVDb.SqliteFolders,
                                       flags.IOVDb.DatabaseInstance)

    for (fs, detDb, className) in listOfFolderInfoTuple:
        fse= _extractFolder(fs)
        # Add class-name to CondInputLoader (if reqired)
        if className is not None:
            loadFolders.add((className, fse))

        if fse in sqliteFolders:
            msg.warning(f'Reading folder {fs} from local storage, bypassing production database')
            fs+=sqliteFolders[fse]
        elif detDb is not None and fs.find('<db>') == -1:

            if db:  # override database name if provided
                dbName=db
            else:
                dbName = flags.IOVDb.DatabaseInstance
            if detDb in _dblist.keys():
                fs = f'<db>{_dblist[detDb]}/{dbName}</db> {fs}'
            elif os.access(detDb, os.R_OK):
                # Assume slqite file
                fs = f'<db>sqlite://;schema={detDb};dbname={dbName}</db> {fs}'
            elif detDb.startswith("crest_fs:"):
                fs = f'<db>{detDb}</db> {fs}'
            else:
                raise ConfigurationError(f'Error, db shorthand {detDb} not known, nor found as sqlite file')
            # Append database string to folder-name

        if extensible:
            fs = fs + '<extensible/>'

        # Add explicitly given xml-modifiers (like channel-selection)
        fs += modifiers

        # Append (modified) folder-name string to IOVDbSvc Folders property
        folders.append(fs)


    result = IOVDbSvcCfg(flags)
    result.getPrimary().Folders+=folders
    if loadFolders:
        result.getCondAlgo('CondInputLoader').Load |= loadFolders

    if flags.IOVDb.CleanerRingSize > 0:
        #HLT-jobs set IOVDb.CleanerRingSize to 0 to run without the cleaning-service, 
        cleanerSvc = CompFactory.Athena.DelayedConditionsCleanerSvc(RingSize=flags.IOVDb.CleanerRingSize)
        result.addService(cleanerSvc)
        result.addService(CompFactory.Athena.ConditionsCleanerSvc(CleanerSvc=cleanerSvc))


    return result


def addFoldersSplitOnline(flags, detDb, onlineFolders, offlineFolders, className=None, extensible=False, addMCString='_OFL', splitMC=False, tag=None, forceDb=None, modifiers=''):
    """Add access to given folder, using either online_folder  or offline_folder. For MC, add addMCString as a postfix (default is _OFL)"""

    if flags.Common.isOnline and not flags.Input.isMC:
        folders = onlineFolders
    elif splitMC and not flags.Input.isMC:
        folders = onlineFolders
    else:
        # MC, so add addMCString
        detDb = detDb + addMCString
        folders = offlineFolders

    return addFolders(flags, folders, detDb, className, extensible, tag=tag, db=forceDb, modifiers=modifiers)


_dblist = {
    'INDET':'COOLONL_INDET',
    'INDET_ONL':'COOLONL_INDET',
    'PIXEL':'COOLONL_PIXEL',
    'PIXEL_ONL':'COOLONL_PIXEL',
    'SCT':'COOLONL_SCT',
    'SCT_ONL':'COOLONL_SCT',
    'TRT':'COOLONL_TRT',
    'TRT_ONL':'COOLONL_TRT',
    'LAR':'COOLONL_LAR',
    'LAR_ONL':'COOLONL_LAR',
    'TILE':'COOLONL_TILE',
    'TILE_ONL':'COOLONL_TILE',
    'MUON':'COOLONL_MUON',
    'MUON_ONL':'COOLONL_MUON',
    'MUONALIGN':'COOLONL_MUONALIGN',
    'MUONALIGN_ONL':'COOLONL_MUONALIGN',
    'MDT':'COOLONL_MDT',
    'MDT_ONL':'COOLONL_MDT',
    'RPC':'COOLONL_RPC',
    'RPC_ONL':'COOLONL_RPC',
    'TGC':'COOLONL_TGC',
    'TGC_ONL':'COOLONL_TGC',
    'CSC':'COOLONL_CSC',
    'CSC_ONL':'COOLONL_CSC',
    'TDAQ':'COOLONL_TDAQ',
    'TDAQ_ONL':'COOLONL_TDAQ',
    'GLOBAL':'COOLONL_GLOBAL',
    'GLOBAL_ONL':'COOLONL_GLOBAL',
    'TRIGGER':'COOLONL_TRIGGER',
    'TRIGGER_ONL':'COOLONL_TRIGGER',
    'CALO':'COOLONL_CALO',
    'CALO_ONL':'COOLONL_CALO',
    'FWD':'COOLONL_FWD',
    'FWD_ONL':'COOLONL_FWD',
    'INDET_OFL':'COOLOFL_INDET',
    'PIXEL_OFL':'COOLOFL_PIXEL',
    'SCT_OFL':'COOLOFL_SCT',
    'TRT_OFL':'COOLOFL_TRT',
    'LAR_OFL':'COOLOFL_LAR',
    'TILE_OFL':'COOLOFL_TILE',
    'MUON_OFL':'COOLOFL_MUON',
    'MUONALIGN_OFL':'COOLOFL_MUONALIGN',
    'MDT_OFL':'COOLOFL_MDT',
    'RPC_OFL':'COOLOFL_RPC',
    'TGC_OFL':'COOLOFL_TGC',
    'CSC_OFL':'COOLOFL_CSC',
    'TDAQ_OFL':'COOLOFL_TDAQ',
    'DCS_OFL':'COOLOFL_DCS',
    'GLOBAL_OFL':'COOLOFL_GLOBAL',
    'TRIGGER_OFL':'COOLOFL_TRIGGER',
    'CALO_OFL':'COOLOFL_CALO',
    'FWD_OFL':'COOLOFL_FWD'
}


def addOverride(flags, folder, tag, tagType="tag", db=None):
    """Add xml override for the specified folder (folder-level tag, forceRunNumber, ...)"""
    suffix = ''
    if db:
        suffix = f' <db>{db}</db>'
    return IOVDbSvcCfg(flags, overrideTags=(f'<prefix>{folder}</prefix> <{tagType}>{tag}</{tagType}>{suffix}',))


def _extractFolder(folderString):
    """Extract the folder name (non-XML text) from a IOVDbSvc.Folders entry"""
    folderName = ''
    xmlTag = ''
    ix = 0
    while ix < len(folderString):
        if (folderString[ix] == '<' and xmlTag == ''):
            ix2 = folderString.find('>', ix)
            if ix2 != -1:
                xmlTag = folderString[ix + 1 : ix2].strip()
                ix = ix2 + 1
        elif folderString[ix:ix+2] == '</' and xmlTag != '':
            ix2 = folderString.find('>', ix)
            if ix2 != -1:
                xmlTag = ''
                ix = ix2 + 1
        else:
            ix2 = folderString.find('<', ix)
            if ix2 == -1:
                ix2 = len(folderString)
            if xmlTag == '':
                folderName = folderName + folderString[ix : ix2]
            ix = ix2
    return folderName.strip()


@cache #Fill only once
def getSqliteContent(sqliteInput,takeFolders,databaseInstance):
    if sqliteInput == "": return dict()
    sqliteFolders=dict()
    if isinstance(takeFolders, str):
        takeFolders=[takeFolders,]
    dbStr="sqlite://;schema="+ sqliteInput+";dbname="+databaseInstance
    from PyCool import cool
    dbSvc = cool.DatabaseSvcFactory.databaseService()
    db = dbSvc.openDatabase(dbStr)
    nodelist=db.listAllNodes()
    for node in nodelist:
        if db.existsFolder(node):
            if (len(takeFolders)>0 and str(node) not in takeFolders): continue
            connStr="<db>"+dbStr+"</db>"
            f=db.getFolder(node)
            if f.versioningMode is not cool.FolderVersioning.SINGLE_VERSION:
                tags=f.listTags()
                if len(tags)==1:
                    connStr+="<tag>"+tags[0]+"</tag>"
            sqliteFolders[str(node)]=connStr
    db.closeDatabase()
    
    if len(takeFolders)>0:
        missedFolders=set(takeFolders)-set(sqliteFolders.keys())
        if len(missedFolders):
            msg.error("The following folders were requested via the flag IOVSvc.sqliteFolder but not found in the sqlite file %s",(sqliteInput))
            for f in missedFolders:
                msg.error(f)
    
    msg.info("The following folders/tags are read from sqlite:")
    for v in sqliteFolders.items():
        msg.info("\t"+str(v))
    return sqliteFolders



@AccumulatorCache #Fill only once
def getCrestDirContent(flags):
    """The CREST version of getSqliteContent. 
    Signficantly more complicated because CREST has no folder (only tags)
    If there is a global tag table defined in the local crest directly, 
    we'll try to use it. 
    Otherwise, open the production DB and guess the tag based on the first part of 
    the folder name. Works only if the tag-naming convention is respected:
    Folder /LAR/ElecCalib/Ramps becomes LARElecCalibRamps-suffix 
    """

    crestDir=flags.IOVDb.SqliteInput
    if crestDir == "": return dict()
    requestedTags=flags.IOVDb.SqliteFolders
    localCrestFolders=dict()

    if len(crestDir.split(":"))==1:
        crestDir="crest_fs:"+crestDir

    missedTags=set(requestedTags) #copy of the tags        

    import chai
    try:
        localdb = chai.Database(crestDir)
    except Exception as e:
        msg.error("Failed to connect to crest directoy %s",crestDir)
        raise e
    
    #see if there is a global-tag defined in the local CREST dir:
    localGTs=[]
    localGT=None
    try:
        localGTs=localdb.find_global_tags()
    except RuntimeError:
        pass

    if len(localGTs)==1:
        localGT=localGTs[0]
        msg.info("Found exactly one global tag in local crest directory: [%s] Try to use it.",localGT)
    elif len(localGTs)>1:
        if flags.IOVDb.GlobalTag in localGTs:
            localGT=flags.IOVDb.GlobalTag
            msg.info("Global tag %s also defined in local crest directory. Try to use it.",flags.IOVDb.GlobalTag)
    else:
        msg.warning("More than one global tag found in crest directory [%s], none matches the global conditions tag %s",
                    crestDir,flags.IOVDb.GlobalTag)
    if localGT:
        gt=localdb.get_global_tag(localGT)
        resolvedTags= gt.resolve_all_tags()
        for gt2ft in resolvedTags.items():
            f=gt2ft[0][0]
            lt=gt2ft[1]
            if len(requestedTags)>0 and lt not in requestedTags: continue
            localCrestFolders[f]="<db>"+crestDir+"</db><ctag>"+lt+"</ctag>"
            missedTags.discard(lt)
    else: 
        #No local tag hierachy defined. Try to make guesses based on tag hierary in the production db:
        msg.warning("No (usable) global tag found in local crest directory %s. Open production db, try to guess folder-tag relation")
        localtags=set(localdb.find_tags())
    
        try:
            proddb=chai.Database("crest:"+flags.IOVDb.DBConnection)
        except Exception as e:
            msg.error("Failed to connect to crest server %s",flags.IOVDb.CrestServer)
            raise e
    
        gt=proddb.get_global_tag(flags.IOVDb.GlobalTag)

        resolvedTags=gt.resolve_all_tags()
        folderstubToFolderMap={}
        tbl=str.maketrans(".","-")
        for gt2ft in resolvedTags.items():
            f=gt2ft[0][0]
            ft=gt2ft[1]
            if ft.startswith("UPGRADE_"): ft=ft[8:] #No idea why we prepend this string ...
            folderstub="".join(f.split("/")).lower()
            tagstub=ft.translate(tbl).split("-")[0].lower()
            if (tagstub != folderstub):
                msg.warning("Folder-tag %s of folder %s in the production DB does not follow tag naming convention",ft, f)
                
            folderstubToFolderMap[folderstub]=f
  
        for lt in localtags:
            if len(requestedTags)>0 and lt not in requestedTags: continue
            tagstub=lt.translate(tbl).split("-")[0].lower()
            if tagstub in folderstubToFolderMap:
                f=folderstubToFolderMap[tagstub]
                localCrestFolders[f]="<db>"+crestDir+"</db><ctag>"+lt+"</ctag>"
                missedTags.discard(lt) 
            else:
                msg.warning("Cannot guess the folder of the tag %s in the local CREST directory %s",lt,crestDir)
                msg.warning("Ignoring this tag")
                    

    msg.info("The following folders/tags are read from local crest directory:")
    for v in localCrestFolders.items():
        msg.info("\t"+str(v))
    if len(missedTags):
        msg.error("The following local Crest tags have been explicitly requested via the flag IOVSvc.sqliteFolder but not found in the local crest directory %s",(crestDir))
        for mt in missedTags:
            msg.error("\t%s",mt)
    
    return localCrestFolders







#post-exec-style helper method to remove a folder from IOVDbSvc.Folders and CondInputLoader.Load
#To be used in calibration-processing jobs that read the condions from anohter source or produce it in the same job
def blockFolder(ca,folder):
        "Block use of specified conditions DB folder so data can be read from elsewhere"
        msg.info("Trying to remove folder [%s] from IOVDbSvc.Folders",folder)
        iovdbsvc=ca.getService("IOVDbSvc")
        oldLen=len(iovdbsvc.Folders)
        iovdbsvc.Folders=[x for x in iovdbsvc.Folders if x.find(folder)==-1]
        newLen=len(iovdbsvc.Folders)
        if (oldLen==newLen):
            msg.warning("Folder [%s] not found in IOVDbSvc.Folder",folder)
            return
        elif (oldLen-newLen>1):
            msg.warning("Folder string [%s] matched more than one folder, removed %i folders",folder,oldLen-newLen)


        condInputLoader=ca.getCondAlgo("CondInputLoader")
        condInputLoader.Load=set([x for x in condInputLoader.Load if x[1].find(folder)==-1])
        return


@cache
def checkGlobalTag(connStr,currGlobalTag):
    fail=False
    if connStr.startswith("http"):
       connStr1="crest:"+connStr
    else: #Assume local file
        connStr1="crest_fs:"+connStr 
    import chai
    try:
        db=chai.Database(connStr1)
        allGlobalTags=set(db.find_global_tags())
    except chai._chai.NotFoundError as e:
        msg.error(str(e))
        msg.error(f"Could not load data from crest URL {connStr}")
        fail=True
    except chai._chai.BackendError as e:
        msg.error(str(e))
        msg.error(f"Could not load data from crest URL {connStr}")
        fail=True

    if fail: raise ConfigurationError()

    if currGlobalTag not in allGlobalTags:
        from difflib import get_close_matches
        m1=get_close_matches(currGlobalTag,allGlobalTags,1)
        msg.error(f"Global tag {currGlobalTag} does not exist"+(f". Did you mean '{m1[0]}'?" if m1 else "")) 
        raise ConfigurationError() 
    del db
    return None




if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = IOVDbSvcCfg(flags)

    with open('test.pkl','wb') as f:
        acc.store(f)
