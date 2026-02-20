# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, tarfile, glob, random, subprocess

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from TRT_CalibAlgs.TRTCalibrationMgrConfig import CalibConfig, TRT_CalibrationMgrCfg, TRT_StrawStatusCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def nextstep(text):
    print("\n"+"#"*100)
    print("#")
    print("#    %s" % (text))
    print("#")
    print("#"*100,"\n")

def tryError(command, error):
    try:
        print(" Running: %s\n" % (command))
        stdout, stderr = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        print("OUTPUT: \n%s" % (stdout.decode('ascii')))
        print("ERRORS: %s" % ("NONE" if stderr.decode('ascii')=='' else "\n"+stderr.decode('ascii')))
        if stderr:
            exit(1)        
    except OSError as e:
        print(error,e)
        sys.exit(e.errno)

def fromRunArgs(runArgs):
    
    outputFile = runArgs.outputTAR_CALIBFile
    
    ##################################################################################################
    nextstep("UNTAR input file")
    ##################################################################################################   
    
    print("Uncompressing files:")
    try:
        for file in runArgs.inputTARFile:
            print("\t-",file)
            tarfile.open(file).extractall(".") 
    except OSError as e:
        print("ERROR: Failed uncompressing TAR file\n",e)
        sys.exit(e.errno)  
    
    ##################################################################################################
    nextstep("Renaming *straw.txt and *tracktuple.root files for the final output")
    ##################################################################################################   
    
    listOfStrawFiles = glob.glob("*.merged.straw.txt")
    listOfTrackFiles = glob.glob("*.tracktuple.root")
    
    if not listOfStrawFiles or (not listOfTrackFiles and runArgs.piecetoken != "all"):
        print("ERROR: Empty array of \"*.merged.straw.txt\" (size %d) or \"*.tracktuple.root\" (size %d) files" % (len(listOfStrawFiles), len(listOfTrackFiles)))
        exit(1)
    
    command  = "mv -v %s %s.merged.straw.txt; " % (listOfStrawFiles[0],outputFile)

    if(runArgs.piecetoken == "all"):
        print("INFO: piecetoken equal to \"all\". Saving *tracktuple.root file")
        command += "mv -v %s %s.tracktuple.root; " % (listOfTrackFiles[0],outputFile)
    else:
        print("INFO: piecetoken different than \"all\". Not need to rename *tracktuple.root file")
    
    tryError(command, "Renaming *straw.txt and *tracktuple.root files\n")
    
    ##################################################################################################
    nextstep("Calculating constants ATHENA")
    ##################################################################################################    
    
    myFile = []
    # generating the RAW file path
    if runArgs.rawfile:
        myFile.append(runArgs.rawfile)
        
    else:
        if not runArgs.project and not runArgs.runnr and not runArgs.stream:
            try:
                splitInput = runArgs.inputTARFile[0].split('.')
                project="----"
                runnr  ="----"
                stream ="----"
                if splitInput:
                    runArgs.project = splitInput[0]
                    runArgs.runnr   = splitInput[1]
                    runArgs.stream  = splitInput[2]
                    print("INFO: No input arguments are given to name the RAW file. Obtained from \"lsn\"...")
                else:
                    print("ERROR: not able to find project=\"%s\" or runNumber=\"%s\" or stream=\"%s\" from \"lsn\" " % (project, runnr, stream))
                    exit(1)
                
            except OSError as e:
                print("ERROR: Failed trying to build RAW file name for Athena.\n",e)
                sys.exit(e.errno)
          
        if (not runArgs.project or not runArgs.runnr or not runArgs.stream):
            print("ERROR: Raw file not provided, project=\"%s\" or runNumber=\"%s\" or stream=\"%s\" missing..." % (runArgs.project, runArgs.runnr, runArgs.stream))
            print("Provide them!")
            sys.exit(1)
        else:

            # the arquitecture behind the path is the one below. The star "*" is different depending on the stream rates
            # for express_express the ending should be "merge.RAW/"
            # for other streams could be "daq.RAW/"
            rawpath = "/eos/atlas/atlastier0/rucio/%s/%s/%s/%s.%s.%s.%s" % (runArgs.project,runArgs.stream,runArgs.runnr.zfill(8),runArgs.project,runArgs.runnr.zfill(8),runArgs.stream,runArgs.step)
            if len(glob.glob(rawpath)) == 0:
                print("ERROR: Any folder under \"%s\". Please check that the path is correct" % rawpath)
                sys.exit(1)  
            elif len(glob.glob(rawpath)) > 1:
                print("ERROR: More than one folder under '%s'. Please specify the correct one with \"step\" argument in the transform" % rawpath)
                for f in glob.glob(rawpath):
                    print(f"\t- {f}")
                sys.exit(1)  

            globedFiles = glob.glob(rawpath+"*")
            if not globedFiles:
                print("ERROR: Not able to find any file under %s. Please check that the files exists" % (rawpath))
                sys.exit(1)            
            
            myFile.append(random.choice(globedFiles))
            print("RAW file selected for testing:",myFile)
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
    flags.dump()
    
    cfg=MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    cfg.merge(InDetTrackRecoCfg(flags))    
    
    cfg.merge(TRT_CalibrationMgrCfg(flags,DoCalibrate=True, Hittuple=glob.glob("*.basic.root")[0], caltag=runArgs.piecetoken))
    cfg.merge(TRT_StrawStatusCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)
    
    with open("ConfigTRTCalib_calib.pkl", "wb") as f: 
        cfg.store(f)    
    
    sc = cfg.run()
    if not sc.isSuccess():
         sys.exit(not sc.isSuccess())
    
    ##################################################################################################
    nextstep("Renaming outputs from Calibrator")
    ##################################################################################################   
    
    command  =  "mv -v calibout.root %s.calibout.root ; " % (outputFile)
    command +=  "mv -v calibout_rt.txt %s.calibout_rt.txt; " % (outputFile)
    command +=  "mv -v calibout_t0.txt %s.calibout_t0.txt; " % (outputFile)
    command +=  "mv -v calib_constants_out.txt %s.calib_constants_out.txt; " % (outputFile)
    
    tryError(command, "ERROR: Failed renaming files in TRT calib step\n")
           
    ##################################################################################################
    nextstep("TAR'ing files")
    ################################################################################################## 
    try:
        # Getting list of files to be compressed
        files_list=glob.glob(outputFile+".*")
        # Compressing
        tar = tarfile.open(outputFile, "w:gz")
        print("Compressing files in %s output file:" % outputFile)
        for file in files_list:
            print("\t-",file)
            tar.add(file)
        tar.close()
    except OSError as e:
        print("ERROR: Failed compressing the output files\n",e)
        sys.exit(e.errno)    

    # Prints all types of txt files present in a Path
    print("\nListing files:")
    for file in sorted(glob.glob("./*", recursive=True)):
        print("\t-",file)            
