#!/usr/bin/env python

#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#####################################################################
# JobOptions for CreateMisalignAlg which creates a misaligned copy
# of the geometry database to introduce misalignments at reconstruction level
# This is configured to run with ITk geometries
##############################################################

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
from InDetAlignGenAlgs.InDetAlignAlgsConfig import CreateITkMisalignAlgCfg
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
from InDetAlignGenTools.InDetAlignGenToolsConfig import ITkAlignDBTool
import sys
from AthenaConfiguration.ComponentFactory import CompFactory

def blockFolder(ca,folder):
        "Block use of specified conditions DB folder so data can be read from elsewhere"
        from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
        ca.merge(IOVDbSvcCfg(flags))
        iovdbsvc=ca.getService("IOVDbSvc")
        # check existing list of folders and remove it if found
        for i in range(0,len(iovdbsvc.Folders)):
            if (iovdbsvc.Folders[i].find(folder)>=0):
                del iovdbsvc.Folders[i]
                break
        condInputLoader=ca.getCondAlgo("CondInputLoader")        
        for i in range(0, len(condInputLoader.Load)):
            if (folder in condInputLoader.Load[i][-1] ):
                del condInputLoader.Load[i]
                break


def getFlags(**kwargs):
    flags=initConfigFlags()
    ## Just enable ID for the moment.
    flags.Input.isMC             = True

    flags.Input.Files = []
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags,defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.GeoModel.Align.Dynamic = False
    
    flags.ITk.Geometry.isAlignable = True

    flags.ITk.Geometry.AllLocal = False
    detectors = [
    "ITkPixel",
    "ITkStrip",
    "Bpipe"
    ]
    setupDetectorFlags(flags, custom_list=detectors, toggle_geometry=True)
    flags.TrackingGeometry.MaterialSource = "Input"


    #Define the output database file name and add it to the flags
    if 'MisalignMode' not in kwargs.keys():
        MisalignMode = 11 # Radial
    else:
        MisalignMode=int(kwargs.get('MisalignMode',11))
    if 'Translation' not in kwargs.keys():
        Translation = True
    else:
        Translation=bool(kwargs.get('Translation','True')=='True')
        if not Translation:
            print ("Translation not set to \"True\" - disabling generation of translations")
    if 'Rotation' not in kwargs.keys():
        Rotation = True
    else:
        Rotation=bool(kwargs.get('Rotation','True')=='True')
        if not Rotation:
            print ("Rotation not set to \"True\" - disabling generation of rotations")
    databaseFilename     = 'MisalignmentSet%s.db' % (MisalignMode)
    flags.IOVDb.DBConnection="sqlite://;schema=%s;dbname=OFLCOND" % (databaseFilename) 
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC


    # This should run serially for the moment.
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.lock()

    return flags

def CreateMis(flags,name="CreateITkMisalignAlg",**kwargs):
    #
    #--------------------------------------------------------------
    # Geometry section
    #--------------------------------------------------------------
    misalignmentOnTopOfExistingSet = False
    readDBPoolFile = False

    createFreshDB = not(readDBPoolFile or misalignmentOnTopOfExistingSet)

    MisalignMode = kwargs.pop('MisalignMode',11)

    shiftInMicrons = 100

    if MisalignMode in [11, 12, 31]:
        shiftInMicrons = 500
    outFiles = 'MisalignmentSet%s' % (MisalignMode)
    misalignModeMap = {0:'no Misalignment',
                    1: 'misalignment by 6 parameters',
                    2: 'random misalignment',
                    3: 'IBL-stave temperature dependent bowing',
                    11: 'R deltaR (radial expansion)', 12: 'Phi deltaR (ellipse)',13: 'Z deltaR (funnel)',
                    21: 'R deltaPhi (curl)', 22: 'Phi deltaPhi (clamshell) ',23:'Z deltaPhi (twist)',
                    31: 'R deltaZ (telescope)',32:'Phi deltaZ (skew)',33:'Z deltaZ (z-expansion)'}
    ####################################################################################################################

    acc=MainServicesCfg(flags)
    print ("\n CreateMisalignAlg: Creation of misalignment mode %s: %s \n" % (int(MisalignMode),misalignModeMap.get(int(MisalignMode),'unknown')))
    kwargs.setdefault("ASCIIFilenameBase",outFiles)
    kwargs.setdefault("SQLiteTag",'MisalignmentMode_'+str(misalignModeMap.get(int(MisalignMode),'unknown')))
    kwargs.setdefault("MisalignMode",int(MisalignMode))
    kwargs.setdefault("Translation",bool(str(kwargs.pop('Translation','True'))=='True'))
    kwargs.setdefault("Rotation",bool(str(kwargs.pop('Rotation','True'))=='True'))
    kwargs.setdefault("MaxShift",shiftInMicrons)
    kwargs.setdefault("CreateFreshDB",createFreshDB)
    #Create and configure the AlignDB tool
    
    outputAlignFolder="/Indet/AlignITk" #The folder name from which initial misalignments should be read - currently may clash if this is the same is the input folder!

    writeDBPoolFile=True   #Activate or deactivate writing to outFiles + '.pool.root'
    kargsTool={}
    kargsTool.setdefault("SCTTwoSide",True)
    kargsTool.setdefault("DBRoot",outputAlignFolder)
    kargsTool.setdefault("DBKey",outputAlignFolder)
    kargsTool.setdefault("forceUserDBConfig",True)
    if writeDBPoolFile:
        print("Writing DB Pool File")
        InDetCondStream=CompFactory.AthenaOutputStreamTool("CondStream_write",OutputFile = outFiles+'.pool.root')
        InDetCondStream.PoolContainerPrefix="<type>"
        InDetCondStream.TopLevelContainerName=""
        InDetCondStream.SubLevelBranchName="<key>"
        kargsTool.setdefault("CondStream",InDetCondStream)
    dbTool = acc.popToolsAndMerge(ITkAlignDBTool(flags,**kargsTool))

    kwargs.setdefault("IDAlignDBTool",dbTool)

    cfg=CreateITkMisalignAlgCfg(flags,name=name,**kwargs)

    acc.merge(cfg)
    if writeDBPoolFile:
        print("To be writen DB pool File")

    return acc

if __name__ == "__main__":
    if(len(sys.argv[1:])):
        kwargs=dict(arg.split('=') for arg in sys.argv[1:])
        print(kwargs)
    else:
        kwargs={}
        print("no args")
    #Get the basic configuration flags
    flags=getFlags(**kwargs)
    flags.dump()
    #Add the tools and the algorithm to the accumulator
    acc=CreateMis(flags,**kwargs)
    acc.printConfig()

    #run
    sc=acc.run(10)
    if sc.isFailure():
        print("Failed to run the Misalignment Algorithm")
        sys.exit(-1)
