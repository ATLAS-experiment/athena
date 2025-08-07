# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, glob, subprocess, tarfile

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
        print("STD::OUTPUT: \n%s" % (stdout.decode('ascii')))
        print("STD::ERRORS: %s" % ("NONE" if stderr.decode('ascii')=='' else "\n"+stderr.decode('ascii')))
        if "error" in stderr.decode('ascii').lower():
            raise RuntimeError(f"Error from subprocess:\n{stderr.decode('ascii')}")
    except RuntimeError as e:
        print(error,e)
        sys.exit(1)

def fromRunArgs(runArgs):
    
    ##################################################################################################
    nextstep("UNTAR files")
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
    nextstep("Generating the tracktuple and StrawStatus file")
    ##################################################################################################
    
    command  = 'TRTCalib_bhadd merged_histo.root %s' % ("".join(("%s " % str(file)) for file in glob.glob("*.basic.root") ))
    tryError(command,"ERROR: Failed in process TRTCalib_bhadd\n")
    
    ##################################################################################################
    nextstep("Renaming Binary output file")
    ##################################################################################################
    
    command = 'mv -v merged_histo.root.part0 %s.basic.root' % (runArgs.outputTAR_MERGEDFile)
    tryError(command,"ERROR: Renaming binary file \"merged_histo.root.part0\"\n")

    ##################################################################################################
    nextstep("Merging *.tracktuple.root files")
    ##################################################################################################
    
    command = 'hadd -f %s.tracktuple.root %s' % (runArgs.outputTAR_MERGEDFile, "".join(("%s " % str(file)) for file in glob.glob("*.tracktuple.root") ))
    tryError(command,"ERROR: Failed in process merging *.tracktuple.root files\n")

    ##################################################################################################
    nextstep("Merging *.straw.txt files")
    ##################################################################################################
    
    command = 'TRTCalib_StrawStatus_merge %s.merged.straw.txt %s' % (runArgs.outputTAR_MERGEDFile, "".join(("%s " % str(file)) for file in glob.glob("*.straw.txt") ))
    tryError(command,"ERROR: Failed in process merging *.straw.txt files\n")
    
    ##################################################################################################
    nextstep("TAR'ing files")
    ################################################################################################## 
       
    try:
        # Getting list of files to be compressed
        files_list=glob.glob(runArgs.outputTAR_MERGEDFile+".*")
        # Compressing
        tar = tarfile.open(runArgs.outputTAR_MERGEDFile, "w:gz")
        print("\nCompressing files in %s output file:" % runArgs.outputTAR_MERGEDFile)
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