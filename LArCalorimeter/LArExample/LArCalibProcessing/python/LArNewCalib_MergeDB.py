# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import os,sys
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders

def DBConnectionFile(sqlitefile):
   return "sqlite://;schema="+sqlitefile+";dbname=CONDBR2"

def mergeDBCfg(flags, InputKeys=[], InputSQLiteFiles=[]):
    result=ComponentAccumulator()

    print("mergeDBCfg",len(InputSQLiteFiles)," SQLite files: ",InputSQLiteFiles)
    print(len(InputKeys)," InputKeys: ",InputKeys)


    if not flags.LArCalib.isSC: GroupingType = "ExtendedSubDetector"
    else:     GroupingType = "SuperCells"

    if ( flags.LArCalib.Output.POOLFile ):
        if os.path.exists(flags.LArCalib.Output.POOLFile): 
            os.remove(flags.LArCalib.Output.POOLFile) 

    for key in InputKeys:

      print("Working for: ",key)
      KeyOutput = key
      FlagInput = key

      if 'Pedestal' == key or 'PhysWave' == key or 'MphysOverMcal' == key or 'Ramp' == key or 'Params' in key:
         KeyInput="LAr"+key
      if 'Pedestal' == key or 'PhysWave' == key or 'CaliWave' or 'MphysOverMcal' == key or 'Ramp' == key or 'Params' in key:
         KeyInput="LAr"+key
         KeyOutput="LAr"+key

      if 'AutoCorr' in key:
         KeyInput='LArAutoCorr'

      ofcphyskey=None
      if "OFC" in key:
         KeyInput = "LArOFC"
         if "Cali" in key:
            FlagInput = "OFCCali"

         if "Phys" in key:
            ofcphyskeytmp = key.split("OFCPhys")[1]
            if 'Mu' in ofcphyskeytmp:
               ofcphyskey=ofcphyskeytmp.replace("Mu","")
            else:   
               ofcphyskey=ofcphyskeytmp
            FlagInput = "OFCPhys"

      shapekey=None
      if "Shape" in key:
         shapekey = key.split("Shape")[1]
         FlagInput = "Shape"
         KeyInput = "LArShape"


      #if FlagInput in flags._flagdict.keys():       
      if   [i for i in flags._flagdict.keys() if FlagInput in i]:
         try: 
            Folder  = flags._get('LArCalib.'+FlagInput+'.Folder')  
         except Exception as e:
            print(e)
            print(flags._flagdict)
            sys.exit(-1)
      else:
         print('No folder in flags for '+FlagInput)
         print(flags._flagdict.keys())
         sys.exit(-2)

      if shapekey is not None:
         if Folder.endswith("/"):
            Folder+=shapekey
         else:
            Folder+="/"+shapekey

      if ofcphyskey is not None and ofcphyskey != "":
         print("Folder: ",Folder," ofcphyskey: ",ofcphyskey)
         if "4samples" in Folder:
            Folder = Folder.replace("4samples", ofcphyskey)
         elif Folder.endswith("/"):
            Folder+=ofcphyskey
         else:
            Folder+="/"+ofcphyskey

      if "OFCPhys" in FlagInput:
         print("Folder: ",Folder)
         # Get the available tags
         from PyCool import cool
         dbSvc=cool.DatabaseSvcFactory.databaseService()
         db_string=DBConnectionFile(InputSQLiteFiles[0])
         try:
            db=dbSvc.openDatabase(db_string)
         except Exception as e:
            print ('Problem opening database',e)
            print(db_string)
            sys.exit(-3)
         cool_folder=db.getFolder(Folder)
         tags = [ x for x in cool_folder.listTags() ]
         if len(tags) == 1:
            TagSpec = str(tags[0])
         else:
            if flags.LArCalib.OFC.Ncoll > 0 or 'Mu' in key:
               TagSpec = [ str(tag) for tag in tags if "-mu-" in str(tag)][0]
            else:
               TagSpec = [ str(tag) for tag in tags if "-mu-" not in str(tag)][0]
      else: 
         from LArCalibProcessing.utils import FolderTagResolver as FolderTagResover
         FolderTagResover._globalTag=flags.IOVDb.GlobalTag
         #rs=FolderTagResover(flags.LArCalib.Input.Database2)
         # until not all folders in COOL:
         rs=FolderTagResover(DBConnectionFile(InputSQLiteFiles[0]))
         FolderTag = rs.getFolderTagSuffix(Folder)
         del rs
 
         from LArCalibProcessing.LArCalibConfigFlags import LArCalibFolderTag
         TagSpec = LArCalibFolderTag(Folder,FolderTag)


      if 'Wave' in KeyInput:
         OutputObjectSpec = KeyInput+"Container#"+KeyOutput+"#"+Folder
      else:   
         OutputObjectSpec = KeyInput+"Complete#"+KeyOutput+"#"+Folder
      OutputObjectSpecTag = TagSpec

      print("**",KeyInput,FlagInput,"folder **",Folder)
      print("** TagSpec: ",TagSpec)
      print("** keyOutput: ",KeyOutput)


      inkeys=[]
      inobjs=[]

      for sqf in InputSQLiteFiles:
         thiskey = KeyInput+str(InputSQLiteFiles.index(sqf))

         if 'PhysAutoCorr' in key: thiskey='Phys'+thiskey

         if 'OFC' in key and 'Mu' in key:
            thiskey+="Mu"
         if 'OFC' in key and 'Cali' in key:
            thiskey+="Cali"   

         inkeys.append(thiskey)
         if 'Wave' in KeyInput: 
            inobjs.append(KeyInput+"Container#"+thiskey)
         else:   
            inobjs.append(KeyInput+"Complete#"+thiskey)

         if 'Wave' in KeyInput or 'Params' in KeyInput:
            result.merge(addFolders(flags,Folder,tag=TagSpec,detDb=sqf,modifiers="<key>"+thiskey+"</key>"))
         else:   
            result.merge(addFolders(flags,Folder,tag=TagSpec,detDb=sqf,modifiers="<key>"+thiskey+"</key>", className=KeyInput+"Complete"))
         pass   
 
      pass
 
      if 'Wave' in KeyInput:
         result.addCondAlgo(CompFactory.getComp('ForceLoadCondObj')(KeyOutput,ObjectList=inobjs))
         result.addCondAlgo(CompFactory.getComp('LArConditionsMergerAlg<LArDAC2uAComplete,'+KeyInput+'Container>')(key+"Merger",
                                              GroupingType = GroupingType, WriteKey = KeyOutput, DetStoreReadKeys = inkeys))    
      elif 'Params' in KeyInput:   
         result.addCondAlgo(CompFactory.getComp('ForceLoadCondObj')(KeyOutput,ObjectList=inobjs))
         result.addCondAlgo(CompFactory.getComp('LArConditionsMergerAlg<LArDAC2uAComplete,'+KeyInput+'Complete>')(key+"Merger",
                                              GroupingType = GroupingType, WriteKey = KeyOutput, DetStoreReadKeys = inkeys))    
      else:
         result.addCondAlgo(CompFactory.getComp('LArConditionsMergerAlg<'+KeyInput+'Complete, LArPhysWaveContainer>')(key+"Merger",
                                              GroupingType = GroupingType, WriteKey = KeyOutput, ReadKeys = inkeys))    

      if ( flags.LArCalib.Output.POOLFile ):

         from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
         result.merge(OutputConditionsAlgCfg(flags,name="OutputConditionsAlg"+KeyInput, outputFile=flags.LArCalib.Output.POOLFile,
                                                       ObjectList=[OutputObjectSpec], IOVTagList=[OutputObjectSpecTag],
                                                       Run1 = flags.LArCalib.IOVStart, Run2= flags.LArCalib.IOVEnd, WriteIOV=True))
    pass

    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))

    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

    result.printConfig(withDetails=True, printDefaults=True)
    return result 


if __name__=="__main__":
    import argparse

    def list_of_strings(arg):
       return arg.split(',')

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r','--run', dest='run', default=str(0x7FFFFFFF), help='Run number to query input DB', type=str)
    parser.add_argument('-i','--insqlite', dest='insql', default="", help='List of input sqlite files, comma separated, if empty all *.db files in current dir.', type=list_of_strings)
    parser.add_argument('-k','--inkeys', dest='inkeys', default=["LArPedestal"], help='List of input keys to process', type=list_of_strings)
    parser.add_argument('-o','--outsqlite', dest='outsql', default="freshConstants_merged.db", help='Output sqlite file', type=str)
    parser.add_argument('-p','--poolfile', dest='poolfile', default="freshConstants_pp.pool.root", help='Output pool file', type=str)
    parser.add_argument('-d','--dbname', dest='dbname', default="COOLOFL_LAR/CONDBR2", help='DB to query for tags', type=str)
    parser.add_argument('--iovstart',dest="iovstart", default=0, help="IOV start (run-number)", type=int)
    parser.add_argument('--isSC', dest='supercells', default=False, help='is SC data ?', action='store_true')
    parser.add_argument('-c','--poolcat', dest='poolcat', default="freshConstants.xml", help='Catalog of POOL files', type=str)
    parser.add_argument('--Ncoll',dest='Ncoll', default=60, help='Number of MinBias collision assumed for OFCs folder', type=int)
    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
        parser.print_help()
        sys.exit(0)

    if len(args.insql)==0:
       InputSQLiteFiles = [ db for db in os.listdir(".") if db.endswith(".db") ]
    else:   
       InputSQLiteFiles = args.insql
 
    if len(InputSQLiteFiles) == 1: #Main readout case, copy first input to output
       import shutil, os
       shutil.copyfile(InputSQLiteFiles[0],args.outsql)   

    #Import the flag-container that is the arguemnt to the configuration methods
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags, args.supercells)

    #Now we set the flags as required for this particular job:
    flags.LArCalib.Input.RunNumbers = [int(args.run),]
    flags.LArCalib.Input.Database = args.insql
    flags.LArCalib.Input.Database2 = args.dbname
       
    flags.LArCalib.Output.POOLFile = args.poolfile
    #flags.LArCalib.Output.ROOTFile2 ="freshConstants2_pp.pool.root"
    flags.IOVDb.DBConnection=DBConnectionFile(args.outsql)

    #The global tag we are working with
    flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"

    flags.Input.Files=[]
    flags.LArCalib.Input.Files = [ ]
    flags.LArCalib.OFC.Ncoll = args.Ncoll
    flags.LArCalib.IOVStart = args.iovstart

    flags.LAr.doAlign=False
    flags.Input.RunNumbers=flags.LArCalib.Input.RunNumbers

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

    flags.Debug.DumpDetStore=True
    flags.Debug.DumpCondStore=True

    flags.lock()
   
    #Import the MainServices (boilerplate)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    #from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    #cfg.merge(PoolReadCfg(flags))
    from AthenaPoolCnvSvc.PoolCommonConfig import PoolSvcCfg
    cfg.merge(PoolSvcCfg(flags))

    cfg.merge(mergeDBCfg(flags,args.inkeys, InputSQLiteFiles))

    cfg.getService("PoolSvc").ReadCatalog+=["xmlcatalog_file:%s"%args.poolcat,]
    cfg.getService("PoolSvc").WriteCatalog="xmlcatalog_file:%s"%args.poolcat
    cfg.getService("IOVDbSvc").DBInstance=""

    cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    #cfg.getService("StoreGateSvc").Dump=True
    #cfg.getService("StoreGateSvc").OutputLevel=2

    cfg.run(1)
