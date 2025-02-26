# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys, os, glob, subprocess, tarfile
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
    
    ##################################################################################################
    nextstep("Building tracks: ATHENA")
    ##################################################################################################      
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags    
    flags=initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    if "TRTCalibAccu_ART_Output" in runArgs.outputTARFile:
        print("WARNING - Testing the TRTCalib_accu transform")
    else:
        print("INFO - Running TRTCalib_accu transform")

    flags.Input.Files=runArgs.inputRAWFile
    flags.Output.HISTFileName=runArgs.outputTARFile

    #Importing flags - Switching off detector parts and monitoring
    CalibConfig(flags)
    
    # To respect --athenaopts 
    flags.fillFromArgs()
    flags.lock()
    flags.dump()
    
    cfg=MainServicesCfg(flags)
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
    
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    cfg.merge(InDetTrackRecoCfg(flags))    
    
    cfg.merge(TRT_CalibrationMgrCfg(flags,calibconstants=runArgs.calibconstants))
    cfg.merge(TRT_StrawStatusCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)
    
    with open("ConfigTRTCalib_accu.pkl", "wb") as f: 
        cfg.store(f)
        
    sc = cfg.run()
    
    if not sc.isSuccess():
         sys.exit(not sc.isSuccess())
        
    outputFile = runArgs.outputTARFile
    ##################################################################################################
    nextstep("Generating the tracktuple and StrawStatus file")
    ##################################################################################################
    
    os.rename(outputFile, outputFile+'.basic.root')
    command = 'TRTCalib_bhadd dumfile %s.basic.root' % (outputFile)
    
    tryError(command,"ERROR: Failed in process TRTCalib_bhadd\n")
    
    ##################################################################################################
    nextstep("Renaming outputs from Athena and TRTCalib_bhadd")
    ##################################################################################################
    
    command  = "mv -v tracktuple.root %s.tracktuple.root ; " % (outputFile) 
    command += "mv -v  %s %s.straw.txt" % (glob.glob('TRT_StrawStatusOutput.*newFormat.txt')[0],outputFile)    
    
    tryError(command,"ERROR: Failed in renaming\n")
        
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
