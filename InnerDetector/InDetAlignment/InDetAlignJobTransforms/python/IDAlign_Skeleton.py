# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignJobTransforms/scripts/IDAlign_Skeleton.py
# Author: David Brunner (david.brunner@cern.ch)

import contextlib
import re
import os

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

from AthenaCommon.Logging import logging
msg = logging.getLogger('IDAlign')

from AthenaConfiguration.Enums import Format, LHCPeriod

# force no legacy job properties
from AthenaCommon import JobProperties
import AthenaCommon.Constants
JobProperties.jobPropertiesDisallowed = True

def getT0SolveDB(runArgs):
    # Check which file to use to extract metadata
    if runArgs.solve and runArgs.iteration > 0:
        outputFile = runArgs.outputConditionFile
        iteration = runArgs.iteration - 1
    
    elif hasattr(runArgs, "outputTFile") and "Block" in runArgs.outputTFile:
        outputFile = runArgs.outputTFile
        iteration = runArgs.iteration - 1

    elif hasattr(runArgs, "outputMonitorFile") and "Block" in runArgs.outputMonitorFile:
        # For monitoring, the iteration DB file to use is the one from the current iteration
        outputFile = runArgs.outputMonitorFile
        iteration = runArgs.iteration
        
    else:
        return ""

    # Extract data taking period, stream, ect from output file name
    try:
        meta_data = re.search(r"^(data.*?_.*?)\.(\d+)\.(\w+).*?(c\d+.*?).*?(Block\d+)", outputFile)
        data_period, run, data_stream, AMI_tag, block = meta_data.groups()
    
    except Exception:
        raise Exception(f"Can not extract metadata from: {outputFile}")
        
    # Try to find local database file
    localDatabaseWildcard = f"{runArgs.eosT0Dir}/{data_period}/{data_stream}/{run}/{data_period}.{run}.{data_stream}.idalignsolve.ROOT_DB.Iter{iteration}*/*{block}*"
    
    try:
        from glob import glob

        # Get always latest DB files, in case a job restarted with a new AMI tag
        def get_AMI_Tag(file_name):
            m = re.search(r'c\d+', file_name)
            return m.group(0) if m else "c0000"

        localDataBases = sorted(glob(localDatabaseWildcard), key = get_AMI_Tag, reverse = True)
        latestLocalDataBase = localDataBases[0]
    
    except Exception:
        raise Exception(f"Could not find local database from wildcard: {localDatabaseWildcard}")
    
    return latestLocalDataBase



def configureInDetFlags(runArgs, flags):

    print("configure the InDet")
    
    ## Turn off ID parts if wished (may cause conflicts with level setting)
    for IDpart in runArgs.excludeIDPart:
        setattr(flags.InDet.Align, f"align{IDpart}", False)

    ## Determine if local database should be used (whether from local exeuction or on T0)
    if hasattr(runArgs, "localDatabase"): 
        flags.InDet.Align.localDataBase = os.path.abspath(runArgs.localDatabase)
    
    if runArgs.eosT0Dir != "":
        flags.InDet.Align.localDataBase = getT0SolveDB(runArgs)
        
    else:
        flags.InDet.Align.localDataBase = ""

    ## Set Tags
    for tag in [tag for tag in dir(runArgs) if "Tag" in tag and tag != "globalTag"]:
        setattr(flags.InDet.Align, tag, getattr(runArgs, tag))

    if flags.InDet.Align.localDataBase:
        msg.info(f"Change IBLDist tag from '{flags.InDet.Align.IBLDistTag}' to 'InDetAlignIBLDIST-T0-Alignment'")
        msg.info(f"Change L1IDTag tag from '{flags.InDet.Align.L1IDTag}' to 'InDetAlignL1-T0-Alignment'")
        msg.info(f"Change L2PIXTag tag from '{flags.InDet.Align.L2PIXTag}' to 'InDetAlignL2PIX-T0-Alignment'")
        msg.info(f"Change L2SCTTag tag from '{flags.InDet.Align.L2SCTTag}' to 'InDetAlignL2SCT-T0-Alignment'")
        msg.info(f"Change L1TRTTag tag from '{flags.InDet.Align.L1TRTTag}' to 'InDetAlignL1TRT-T0-Alignment'")

        flags.InDet.Align.IBLDistTag = "InDetAlignIBLDIST-T0-Alignment"
        flags.InDet.Align.L1IDTag = "InDetAlignL1-T0-Alignment"
        flags.InDet.Align.L2PIXTag = "InDetAlignL2PIX-T0-Alignment"
        flags.InDet.Align.L2SCTTag = "InDetAlignL2SCT-T0-Alignment"
        flags.InDet.Align.L1TRTTag = "InDetAlignL1TRT-T0-Alignment"

    ## Set configuration for chosen alignment level
    from InDetAlignConfig.IDAlignFlags import setL11AlignmentFlags, setL16AlignmentFlags, setL2AlignmentFlags, setL3AlignmentFlags

    if runArgs.alignLevel == 11:
        setL11AlignmentFlags(flags)
       
    elif runArgs.alignLevel == 16:
        setL16AlignmentFlags(flags)
        
    elif runArgs.alignLevel == 2:
        setL2AlignmentFlags(flags)
        
    elif runArgs.alignLevel == 3:
        setL3AlignmentFlags(flags)

    else:
        raise Exception(f"No valid alignment level has been selected: '{runArgs.alignLevel}'")

    ## Disable all non-track related flag parameter
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

    ## Update flags based on parser line args
    flags.InDet.Align.accumulate = runArgs.accumulate
    flags.InDet.Align.baseDir = os.path.abspath(runArgs.baseDir)
    flags.InDet.Align.inputTracksCollection = runArgs.inputTracksCollection
    flags.Input.Files = [os.path.abspath(inputFile) for inputFile in runArgs.inputRAWFile]
    
    if runArgs.accumulate:
        if hasattr(runArgs, "outputTFile"):
            flags.InDet.Align.outputTFile = runArgs.outputTFile
    
        if hasattr(runArgs, "outputMonitorFile"):
            flags.InDet.Align.doMonitoring = True
            flags.Output.HISTFileName = f"{flags.InDet.Align.baseDir}/Accumulate/{runArgs.outputMonitorFile}"
    
    if runArgs.solve:
        flags.InDet.Align.inputTFiles = [os.path.abspath(inputTFile) for inputTFile in runArgs.inputTFile]
        flags.InDet.Align.outputConditionFile = f"{flags.InDet.Align.baseDir}/Solve/{runArgs.outputConditionFile}"
        flags.IOVDb.DBConnection = f"sqlite://;schema={flags.InDet.Align.baseDir}/Solve/{runArgs.outputDBFile};dbname=CONDBR2"

    flags.Exec.MaxEvents = runArgs.maxEvents if not runArgs.solve else 1
    flags.Exec.SkipEvents = runArgs.skipEvents if hasattr(runArgs, "skipEvents") else 0   
    flags.Exec.OutputLevel = getattr(AthenaCommon.Constants, runArgs.logLevel)
    flags.Exec.FPE = -2
    flags.IOVDb.GlobalTag = runArgs.globalTag
        
    # flags.GeoModel.Align.Dynamic = True
    # flags.GeoModel.AtlasVersion = runArgs.atlasVersion

    if not flags.Input.isMC and runArgs.isCosmics:
        from AthenaConfiguration.Enums import BeamType
        
        flags.Beam.NumberOfCollisions = 0
        flags.Beam.Type = BeamType.Cosmics
        flags.Beam.Energy = 0.
        flags.Beam.BunchSpacing = 50

    if runArgs.isHeavyIon:
        flags.Beam.BunchSpacing = 50
        flags.Reco.EnableHI = True
        flags.HeavyIon.doGlobal = True
          
    else:
        flags.Beam.BunchSpacing = 25
                
    if not runArgs.isBFieldOff:
        flags.BField.solenoidOn = True
        flags.BField.barrelToroidOn = True
        flags.BField.endcapToroidOn = True
            
    else:
        flags.BField.solenoidOn = False
        flags.BField.barrelToroidOn = False
        flags.BField.endcapToroidOn = False

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    return flags

def configureITkFlags(runArgs, flags):

    ## Set Tags
    for tag in [tag for tag in dir(runArgs) if "Tag" in tag and tag != "globalTag"]:
        setattr(flags.InDet.Align, tag, getattr(runArgs, tag))

    ## Set configuration for chosen alignment level
    from InDetAlignConfig.IDAlignFlags import setL3AlignmentFlags
        
    if runArgs.alignLevel == 3:
        setL3AlignmentFlags(flags)

    else:
        raise Exception(f"No valid alignment level has been selected: '{runArgs.alignLevel}'")


    ## Disable all non-track related flag parameter
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)


    flags.ITk.Align.alignITk = runArgs.alignITk
    flags.ITk.Align.alignITkPixel = runArgs.alignITkPixel
    flags.ITk.Align.alignITkStrip = runArgs.alignITkStrip

    flags.ITk.Align.writeSilicon = False

    flags.ITk.Align.inputTFiles = runArgs.inputTFiles

    if runArgs.localgeo:
        flags.ITk.Geometry.AllLocal = True

    if runArgs.localDB:
        flags.ITk.Align.useLocalDatabase = True

        flags.IOVDb.DBConnection = (
            f"sqlite://;schema={runArgs.localDB};dbname=OFLCOND"
        )

        flags.ITk.Geometry.alignmentFolder = "/Indet/AlignITk"

    if flags.ITk.Align.alignITkPixel:
        flags.ITk.Geometry.pixelAlignable = True

    if flags.ITk.Align.alignITkStrip:
        flags.ITk.Geometry.stripAlignable = True

    # Lock flags
    flags.lock()
    return flags
    

def configureFlags(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.GeoModel.Align.Dynamic = True
    flags.GeoModel.AtlasVersion = runArgs.atlasVersion
    
    if flags.GeoModel.Run > LHCPeriod.Run3:
        # needs to be adapted
        return configureInDetFlags(runArgs, flags)
    else: 
        # return configureITkFlags(runArgs, flags)
        return configureInDetFlags(runArgs, flags)

    

def fromRunArgs(runArgs):
    flags = configureFlags(runArgs)

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    if flags.Input.Format is Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags))

    elif flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    ## Reconstruction related cfg
    with open(os.devnull, 'w') as f, contextlib.redirect_stdout(f):
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        cfg.merge(InDetTrackRecoCfg(flags))

    ## Accumulate step
    if runArgs.accumulate and not runArgs.solve:
        os.makedirs(f"{flags.InDet.Align.baseDir}/Accumulate", exist_ok = True)
        os.chdir(f"{flags.InDet.Align.baseDir}/Accumulate")
        from InDetAlignConfig.AccumulateConfig import AccumulateCfg
        cfg.merge(AccumulateCfg(flags))

    ## Solve step
    elif runArgs.solve and not runArgs.accumulate:
        os.makedirs(f"{flags.InDet.Align.baseDir}/Solve", exist_ok = True)
        os.chdir(f"{flags.InDet.Align.baseDir}/Solve")
        from InDetAlignConfig.SolveConfig import SolveCfg
        cfg.merge(SolveCfg(flags))
               
    else:
        raise Exception("You can run either the acculumation step or the solve step, but not both or neither at the same time!")

    ## Update condition database (Needs to be done last)
    if flags.GeoModel.Run > LHCPeriod.Run3:
        # This is what needs to be corrected
        from InDetAlignConfig.IDAlignConditionConfig import UpdateTagsCfg
        cfg.merge(UpdateTagsCfg(flags))
        # flags.ITk.Align.alignITkPixel = kwargs
    else:
        from InDetAlignConfig.IDAlignConditionConfig import UpdateTagsCfg
        cfg.merge(UpdateTagsCfg(flags))

    ## Post-include
    processPostInclude(runArgs, flags, cfg)

    ## Post-exec
    processPostExec(runArgs, flags, cfg)

    ## Run the setup       
    if runArgs.dryRun:
        sc = cfg.printConfig(summariseProps = True)
       
    else:
        sc = cfg.run()
        
    ## Tar zip log files if wished
    if hasattr(runArgs, "outputTaredLogFile"):
        from glob import glob 
        import tarfile
        
        filesToTar = glob(f"{flags.InDet.Align.baseDir}/Solve/Old*") + glob(f"{flags.InDet.Align.baseDir}/Solve/Output*") + [f"{flags.InDet.Align.baseDir}/Solve/alignlogfile.txt"]
        
        with tarfile.open(f'{flags.InDet.Align.baseDir}/Solve/{runArgs.outputTaredLogFile}', "w:gz") as tar:
            for fileName in filesToTar:
                tar.add(fileName, arcname = fileName.split('/')[-1])
        
    ## If on Tier0, output files need to be in base dir
    if runArgs.eosT0Dir != "":
        from glob import glob
    
        for file_name in glob(f"{flags.InDet.Align.baseDir}/Accumulate/*" if runArgs.accumulate else f"{flags.InDet.Align.baseDir}/Solve/*"):
            os.system(f"mv -fv {file_name} {flags.InDet.Align.baseDir}")
        
    import sys
    sys.exit(sc.isFailure())
