# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, os, glob, subprocess, tarfile

def fromRunArgs(runArgs):
    
    ##################################################################################################
    #
    #       UNTAR files
    #
    ##################################################################################################
    
    print("\nUncompressing files:")
    try:
        for file in runArgs.inputTARFile:
            print("\t-",file)
            tarfile.open(file).extractall(".") 
    except OSError as e:
        print("ERROR: Failed uncompressing TAR file\n",e)
        sys.exit(e.errno)    
    
    ##################################################################################################
    #
    #       Generating the tracktuple and StrawStatus file
    #
    ##################################################################################################
    
    try:
        command = 'TRTCalib_bhadd merged_histo.root %s > TRTCalib_bhadd.log' % ("".join(("%s.basic.root " % str(file)) for file in runArgs.inputTARFile ))
        print("\n Running:",command)
        stdout, stderr = subprocess.Popen(command, shell=True).communicate()
        # Renaming the output of TRTCalib_bhadd
        os.rename("merged_histo.root.part0","%s.basic.root" % (runArgs.outputTAR_MERGEDFile))
    except OSError as e:
        print("ERROR: Failed in process TRTCalib_bhadd\n",e)
        sys.exit(e.errno)

    ##################################################################################################
    #
    #       Merging *.tracktuple.root files
    #
    ##################################################################################################
    
    try:
        command = 'hadd -f %s.tracktuple.root %s > hadd_tracktuple.log' % (runArgs.outputTAR_MERGEDFile, "".join(("%s.tracktuple.root " % str(file)) for file in runArgs.inputTARFile ))
        print("\n Running:",command)
        stdout, stderr = subprocess.Popen(command, shell=True).communicate()
    except OSError as e:
        print("ERROR: Failed in process merging *.tracktuple.root files\n",e)
        sys.exit(e.errno)

    ##################################################################################################
    #
    #       Merging *.straw.txt files
    #
    ##################################################################################################
    
    try:
        command = 'TRTCalib_StrawStatus_merge %s.merged.straw.txt %s > TRT_StrawStatus_merge.log' % (runArgs.outputTAR_MERGEDFile, "".join(("%s.straw.txt " % str(file)) for file in runArgs.inputTARFile ))
        print("\n Running:",command)
        stdout, stderr = subprocess.Popen(command, shell=True).communicate()
    except OSError as e:
        print("ERROR: Failed in process merging *.straw.txt files\n",e)
        sys.exit(e.errno)
    
        
    ##################################################################################################
    #
    #       Compressing outputs in a tar file!
    #
    ################################################################################################## 
       
    # WORK IN PROGRESS  !!!! 
    # try:
    #     # Getting list of files to be compressed
    #     files_list=glob.glob(outputFile+"*")
    #     # Compressing
    #     tar = tarfile.open(outputFile, "w:gz")
    #     print("\nCompressing files in %s output file:" % outputFile)
    #     for file in files_list:
    #         print("\t-",file)
    #         tar.add(file)
    #     tar.close()
    # except OSError as e:
    #     print("ERROR: Failed compressing the output files\n",e)
    #     sys.exit(e.errno)        
            
    
    # Prints all types of txt files present in a Path
    print("\nListing files:")
    for file in glob.glob("./*", recursive=True):
        print("\t-",file)    