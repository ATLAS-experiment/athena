# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#
# The AnalysisApp module assists with the creation of analysis-type applications for athena
# Here is a skeleton application (script) that uses AnalysisApp:
#
# if __name__=="__main__":
#     from AthenaConfiguration import AnalysisApp
#     from AthenaConfiguration.ComponentFactory import CompFactory
#     flags = AnalysisApp.initFlags()                                   # creates a flag container (AthConfigFlags) appropriate for analysis uses
#     flags.addFlag("MyPackage.myArg", "whatever", help="A demo flag")  # user flag example
#     flags.parser().add_argument('--someArg',default="whatever")       # user arg example
#     ca = AnalysisApp.initCfg(flags)                                   # locks the flags, which triggers arg parsing
#     ca.addEventAlgo( CompFactory.MyPackageAlg(MyProperty = flags.MyPackage.myArg), sequenceName="AthAlgSeq" ) # example alg configuring
#     AnalysisApp.launch(flags,ca)                                      # runs the job
#
# If this code is located in MyPackage/python/MyModule.py you can run it with:
#     python -m MyPackage.MyModule [options] [flags]
# Pass the "--help" option to see available options (both pre-defined and user-defined)


def initFlags():
    """
    Creates a flag container (AthConfigFlags) appropriate for typical
    Analysis athena applications.
    :return: flags
    """
    from AthenaConfiguration import AthConfigFlags
    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    from Campaigns.Utils import Campaign
    acf=AthConfigFlags.AthConfigFlags()

    #Flags steering the job execution:
    from AthenaCommon.Constants import INFO
    from AthenaConfiguration.Enums import ProductionStep
    import argparse
    acf.addFlag('Exec.OutputLevel',INFO) #Global Output Level
    acf.addFlag('Exec.MaxEvents',-1)
    acf.addFlag('Exec.SkipEvents',0)
    acf.addFlag('Exec.DebugStage','', help=argparse.SUPPRESS)
    acf.addFlag('Exec.FPE',-2) #-2: No FPE check at all, -1: Abort with core-dump, 0: FPE Auditor w/o stack-tace (default) , >0: number of stack-trace printed by the job


    #Custom messaging for components, see Utils.setupLoggingLevels
    acf.addFlag('Exec.VerboseMessageComponents',[])
    acf.addFlag('Exec.DebugMessageComponents',[])
    acf.addFlag('Exec.InfoMessageComponents',[])
    acf.addFlag('Exec.WarningMessageComponents',[])
    acf.addFlag('Exec.ErrorMessageComponents',[])

    acf.addFlag('Common.MsgSuppression',True) # Enable suppression of printout in MessageSvc
    acf.addFlag('Common.MsgSourceLength',50) #Length of the source-field in the format str of MessageSvc
    acf.addFlag('Common.ProductionStep', ProductionStep.Default, enum=ProductionStep, help=argparse.SUPPRESS)
    acf.addFlag('Common.isOverlay', False, help=argparse.SUPPRESS)

    #Flags describing the input data
    acf.addFlag('Input.Files', ["_ATHENA_GENERIC_INPUTFILE_NAME_",]) # former global.InputFiles
    acf.addFlag('Input.OverrideRunNumber', False, help=argparse.SUPPRESS )
    acf.addFlag('Input.SecondaryFiles', [], help=argparse.SUPPRESS) # secondary input files for DoubleEventSelector
    acf.addFlag('Input.ProcessingTags', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("processingTags", []), help="expert flag, do not override" ) # list of names of streams written to this file
    acf.addFlag('Input.ProjectName', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("project_name", "data17_13TeV"), help="expert flag, do not override") # former global.ProjectName
    acf.addFlag('Input.MCCampaign', lambda prevFlags : Campaign(GetFileMD(prevFlags.Input.Files).get("mc_campaign", "")), enum=Campaign, help="expert flag, do not override")


    acf.addFlag('Concurrency.NumProcs', 0, help="0 = disables MP, otherwise is # of processes to use in MP mode")
    acf.addFlag('Concurrency.NumThreads', 0, help="0 = disables MT, otherwise is # of threads to use in MT mode" )
    acf.addFlag('Concurrency.NumConcurrentEvents', lambda prevFlags : prevFlags.Concurrency.NumThreads)
    acf.addFlag('Concurrency.DebugWorkers', False )

    # output
    acf.addFlag('Output.HISTOutputs', [],help="ROOT output files. Specify in form of 'STREAM:filename.root'")
    acf.addFlag('Output.TreeAutoFlush', {}, help="{} = automatic for all streams, otherwise {'STREAM': 123}")

    acf.addFlag("PoolSvc.MaxFilesOpen", 0, help=argparse.SUPPRESS)

    # analysis-specific arguments
    acf.parser().add_argument('--accessMode',default="POOLAccess",choices={"POOLAccess","ClassAccess"},help="Input file reading mode") # named arg
    acf.parser().add_argument('--postExec',default=None,help="Any postconfig execution required")


    return acf

def initCfg(flags):
    """
    Creates a ComponentAccumulator appropriate for typical Analysis
    type jobs.
    :param flags:
    :return:
    """
    flags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory

    ca = MainServicesCfg(flags)

    if flags.args().accessMode == "POOLAccess":
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        ca.merge(PoolReadCfg(flags))
    else:
        from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg,xAODAccessMode
        ca.merge(xAODReadCfg(flags, AccessMode = xAODAccessMode.CLASS_ACCESS))

    outputs = ["{} DATAFILE='{}' OPT='RECREATE'".format(*file.split(":",1)) for file in flags.Output.HISTOutputs]
    if len(outputs): ca.addService(CompFactory.THistSvc(Output = outputs))

    ca.getService("MessageSvc").setWarning += ["ClassIDSvc","PoolSvc","AthDictLoaderSvc","AthenaPoolAddressProviderSvc",
                                               "ProxyProviderSvc","DBReplicaSvc","MetaDataSvc","MetaDataStore","AthenaPoolCnvSvc",
                                                "TagMetaDataStore","EventSelector",
                                                #"ApplicationMgr", can't silence because otherwise ATN tests fail, see ATLINFR-1235
                                                "CoreDumpSvc","AthMasterSeq","EventPersistencySvc","ActiveStoreSvc",
                                                "AthenaEventLoopMgr","AthOutSeq","AthRegSeq"]

    return ca

def launch(flags,ca):
    """
    Launches the job (includes executing any postExec)
    :param flags:
    :param ca:
    :return:
    """
    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags,ca)
    if flags.args().postExec: eval(flags.args().postExec)
    if ca.run().isFailure():
       import sys
       sys.exit(1)

