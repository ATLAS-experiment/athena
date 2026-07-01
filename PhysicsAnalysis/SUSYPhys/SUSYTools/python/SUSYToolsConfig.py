#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.Logging import log

def SUSYToolsAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    
    isMC       = flags.Input.isMC
    isFastSim  = flags.Sim.ISF.Simulator.usesFastCaloSim() #full sim or atlfast

    # configure SUSYTools algorithm and its tool
    SUSYToolsAlg = CompFactory.SUSYToolsAlg
    alg = SUSYToolsAlg(
        "SUSYToolsAlg",
        DoSyst = isMC and (not susyArgs.noSyst)
    )

    if susyArgs.configFile:
        alg.SUSYTools.ConfigFile = susyArgs.configFile
    else:
        # select config file based on whether we are run3 or run2
        if flags.GeoModel.Run is LHCPeriod.Run3:
            alg.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default_Run3.conf" # run3
        else:
            alg.SUSYTools.ConfigFile = "SUSYTools/SUSYTools_Default.conf"  # run2

        if susyArgs.testFormat == "PHYSLITE":
            STconfig_lite = str(alg.SUSYTools.ConfigFile).replace(".conf","_LITE.conf")
            alg.SUSYTools.IsPHYSLITE = True
            alg.SUSYTools.ConfigFile = STconfig_lite

    log.info("Configuration file: %s",alg.SUSYTools.ConfigFile)

    alg.SUSYTools.DataSource = 0 if not isMC else (1 if not isFastSim else 2) # data/FS/atlfast

    log.info("Configuration SUSYTools.DataSource: %s",alg.SUSYTools.DataSource)

    if isMC:
        if susyArgs.prwFiles:
            alg.SUSYTools.PRWConfigFiles = susyArgs.prwFiles
        else:
            alg.SUSYTools.AutoconfigurePRWTool = True
            alg.SUSYTools.PRWUseCommonMCFiles = True
        # set lumicalc info based on the campaign, if running on MC
        if susyArgs.lumicalcFiles:
            alg.SUSYTools.PRWLumiCalcFiles = susyArgs.lumicalcFiles
        else:
            from PileupReweighting.AutoconfigurePRW import getLumicalcFiles
            alg.SUSYTools.PRWLumiCalcFiles = getLumicalcFiles(flags.Input.MCCampaign)


    acc.addEventAlgo(alg)
    
    return acc

if __name__ == "__main__": # typically not needed in top level script

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    # 1) Init flags
    flags = initConfigFlags()
    
    # Standard argparse.ArgumentParser initialisation options can be provided here
    susyArgsParser = flags.getArgumentParser( )
    susyArgsParser.add_argument("--testCampaign",action="store",default=None,choices=["mc20e","mc23a","mc23d","data23","data22","data18"],help="Specify to select a test campaign")
    susyArgsParser.add_argument("--testFormat",action="store",default="PHYS",choices=["PHYS","PHYSLITE"],help="Specify to select a test format")
    susyArgsParser.add_argument("--accessMode",action="store",choices=["POOLAccess","ClassAccess"],default="POOLAccess",help="xAOD read mode - Class is faster, POOL is more robust")
    susyArgsParser.add_argument("--configFile",action="store",default=None,help="Name of the SUSYTools config file, leave blank for auto-config")
    susyArgsParser.add_argument("--prwFiles",action="store",nargs="+",default=None,help="Name of prw files")
    susyArgsParser.add_argument("--lumicalcFiles",action="store",nargs="+",default=None,help="Name of lumicalc files")
    susyArgsParser.add_argument("--noSyst",action="store_true",help="include to disable systematics")
    susyArgsParser.add_argument("--fileOutput",default=None,help="Name of output file")

    
    susyArgs = flags.fillFromArgs(parser=susyArgsParser)

    if susyArgs.testCampaign:
        pTag = 'p6269' if ('data2' in susyArgs.testCampaign) else 'p6266'
        inputDir = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools'
        inputFiles = {}
        inputFiles['data18'] = f'data18_13TeV.39757132_{pTag}.{susyArgs.testFormat}.pool.root'
        inputFiles['data22'] = f'data22_13p6TeV.39672246_{pTag}.{susyArgs.testFormat}.pool.root'
        inputFiles['data23'] = f'data23_13p6TeV.39756993_{pTag}.{susyArgs.testFormat}.pool.root'
        inputFiles['mc20e']  = f'DAOD_{susyArgs.testFormat}.mc20_13TeV.410470.FS_mc20e_{pTag}.{susyArgs.testFormat}.pool.root'
        inputFiles['mc23a']  = f'mc23_13p6TeV.601229.FS_mc23a_{pTag}.{susyArgs.testFormat}.pool.root'
        inputFiles['mc23d']  = f'mc23_13p6TeV.601229.FS_mc23d_{pTag}.{susyArgs.testFormat}.pool.root'
        flags.Input.Files = [f'{inputDir}/{inputFiles[susyArgs.testCampaign]}']
        if susyArgs.fileOutput is None: 
            susyArgs.fileOutput = f"hist-Ath_{susyArgs.testCampaign}_DAOD_{susyArgs.testFormat}.root"

    log.info("Processing: %s",flags.Input.Files)
    log.info("Outputting: %s",susyArgs.fileOutput)
    
    flags.lock()
    
    acc = MainServicesCfg(flags)
    acc.merge(SUSYToolsAlgCfg(flags))

    status = acc.run()

    import sys
    sys.exit(not status.isSuccess())


