# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, os, tarfile, glob, random

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from TRT_CalibAlgs.TRTCalibrationMgrConfig import CalibConfig, TRT_CalibrationMgrCfg, TRT_StrawStatusCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def fromRunArgs(runArgs):
    
    myFile = []
    # generating the RAW file path
    if runArgs.rawfile:
        myFile.append(runArgs.rawfile)
    elif not runArgs.project or not runArgs.runnr or not runArgs.stream:
        # This part is under testing, will be further developed
        print("ERROR: project=\"%s\" or runNumber=\"%s\" or stream=\"%s\" missing..." % (runArgs.project, runArgs.runnr, runArgs.stream))
        print("Provide them!")
        sys.exit(1)
    else:
        rawpath = "/eos/atlas/atlastier0/rucio/%s/%s/%s/%s.%s.%s.merge.RAW/" % (runArgs.project,runArgs.stream,runArgs.runnr.zfill(8),runArgs.project,runArgs.runnr.zfill(8),runArgs.stream)
        myFile.append(random.choice(glob.glob(rawpath+"*")))
        if not myFile:
            print("ERROR: provide a valid project=\"%s\" or runNumber=\"%s\" or stream=\"%s\"" % (runArgs.project, runArgs.runnr, runArgs.stream))
            sys.exit(1)
    
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags    
    flags=initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)
    
    # Change this to a the hardcoded data file
    flags.Input.Files=myFile
    flags.Output.HISTFileName="DontUse.remove"
    
    # Only one event must be run 
    flags.Exec.MaxEvents=1
    
    # Importing flags - Switching off detector parts and monitoring
    CalibConfig(flags)
    
    # To respect --athenaopts 
    flags.fillFromArgs()
    
    # Reason why we need to clone and replace: https://gitlab.cern.ch/atlas/athena/-/merge_requests/68616#note_7614858
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass",
        # Keep original flags as some of the subsequent passes use
        # lambda functions relying on them
        keepOriginal=True)      

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    cfg.merge(InDetTrackRecoCfg(flags))    
    
    cfg.merge(TRT_CalibrationMgrCfg(flags,DoCalibrate=True, Hittuple=runArgs.inputTARFile[0], caltag=runArgs.piecetoken))
    cfg.merge(TRT_StrawStatusCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)
    
    with open("ConfigTRTCalib_calib.pkl", "wb") as f: 
        cfg.store(f)    
    
    sc = cfg.run()
    if not sc.isSuccess():
         sys.exit(not sc.isSuccess())
         
    outputFile = runArgs.outputTAR_CALIBFile
    
    ##################################################################################################
    #
    #       Renaming outputs from Calibrator
    #
    ##################################################################################################    
    try:
        print("\tRunning: mv calibout.root %s.calibout.root" % outputFile)
        os.rename('calibout.root'          , outputFile+'.calibout.root')
        print("\tRunning: mv calibout_rt.txt %s.calibout_rt.txt" % outputFile)
        os.rename('calibout_rt.txt'        , outputFile+'.calibout_rt.txt')
        print("\tRunning: mv calibout_t0.txt %s.calibout_t0.txt" % outputFile)
        os.rename('calibout_t0.txt'        , outputFile+'.calibout_t0.txt')
        print("\tRunning: mv calib_constants_out.txt %s.calib_constants_out.txt" % outputFile)
        os.rename('calib_constants_out.txt', outputFile+'.calib_constants_out.txt')
    except OSError as e:
        print("ERROR: Failed renaming files in TRT calib step\n",e)
        sys.exit(100) 
        
           
    ##################################################################################################
    #
    #       Compressing outputs in a tar file!
    #
    ################################################################################################## 
    try:
        # Getting list of files to be compressed
        files_list=glob.glob(outputFile+"*")
        # Compressing
        tar = tarfile.open(outputFile, "w:gz")
        print("\nCompressing files in %s output file:" % outputFile)
        for file in files_list:
            print("\t-",file)
            tar.add(file)
        tar.close()
    except OSError as e:
        print("ERROR: Failed compressing the output files\n",e)
        sys.exit(101)    
         
         
