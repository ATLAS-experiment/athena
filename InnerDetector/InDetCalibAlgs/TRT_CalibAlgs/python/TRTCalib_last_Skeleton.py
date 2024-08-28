# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys, os, glob, subprocess, tarfile

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
    nextstep("Filtering files")
    ##################################################################################################
    
    # RT files (nor merging barrel)
    files_list_rt =[item for item in glob.glob("*rt.txt") if "barrel" not in item]
    files_list_t0 =[item for item in glob.glob("*t0.txt") if "barrel" not in item]
    files_list_cal=[item for item in glob.glob("*calibout.root") if "barrel" not in item]
    # tracks
    files_list_trk=glob.glob("*tracktuple.root")
    # straws
    files_list_stw=glob.glob("*merged.straw.txt")
    # constants
    files_list_ctn=glob.glob("*calib_constants_out.txt")
    
    def listFiles(v,txt=""):
        print("Number of files for %s: %i" % (txt,len(v)))
        
        if not v:
            print("ERROR: list of files for %s is empty" % (txt) )
            sys.exit(1)
            
        for i in v:
            print("\t-%s: %s" % (txt,i))
            
    listFiles(files_list_rt ,"RT")
    listFiles(files_list_t0 ,"T0")
    listFiles(files_list_cal,"Cal")
    listFiles(files_list_trk,"Track")
    listFiles(files_list_stw,"Straws")
    listFiles(files_list_ctn,"Constants")
    
    ##################################################################################################
    nextstep("Merging Rt and t0 files")
    ##################################################################################################  
    
    command  = "touch calibout.txt ;"  
    command += "  echo '# Fileformat=2' >> calibout.txt ; "
    command += "  echo '# RtRelation'   >> calibout.txt ; "
    
    # Merging Rt files
    for _f in files_list_rt :
        command += "cat %s >> calibout.txt ; " % (_f)
        
    command += "  echo '# StrawT0' >> calibout.txt ; "
    
    # Merging t0 files
    for _f in files_list_t0 :      
        command += "cat %s >> calibout.txt ; " % (_f)

    # add footer
    command += "  echo '#GLOBALOFFSET 0.0000' >> calibout.txt ; "
    
    if os.path.isfile("calibout.txt"):
        print("calibout.txt.. already exists. Removed.")
        os.remove("calibout.txt")
            
    tryError(command,"ERROR: Not able to merge the Rt and t0 files\n") 
      
    
    ##################################################################################################
    nextstep("Merging constants files")
    ##################################################################################################

    command = " touch oldt0s.txt ;"
    for _f in files_list_ctn :
        command += "cat %s >> oldt0s.txt ; " % (_f)    

    if os.path.isfile("oldt0s.txt"):
        print("oldt0s.txt.. already exists. Removed.")
        os.remove("oldt0s.txt")

    tryError(command,"ERROR: Not able to merge the Rt and t0 files\n")   
          
    
    ##################################################################################################
    nextstep("Shift text file")
    ##################################################################################################
    
    command = 'TRTCalib_cfilter.py calibout.txt calibout.txt oldt0s.txt shiftrt'  
      
    tryError(command,"ERROR: Failed in process TRTCalib_cfilter\n")    

    ##################################################################################################
    nextstep("Run hadd to merge root files")
    ##################################################################################################
    
    # Removing the output file if it exists to avoid problems
    if os.path.isfile("merge.root"):
        print("merge.root.. already exists. Removed.")
        os.remove("merge.root")  
        
    command = 'hadd merge.root '
    # merge all the files
    for _f in files_list_cal :
        command += "%s " % (_f)     
    # and track tuple, we just need the first one since they are all the same
    command += "%s " % files_list_trk[0]       
    
    tryError(command,"ERROR: Failed in process hadd\n")         
    
    ##################################################################################################
    nextstep("Rename root files")
    ##################################################################################################
    
    outputFile = runArgs.outputTAR_MERGEDFile
    runNumber = int(runArgs.inputTARFile[0].split('.')[1])
    
    # Rename calibout.txt and merge.root
    command  = "mv -v calibout.txt %s.calibout.txt ; " % outputFile 
    command += "mv -v merge.root %s.merge.root ; " % outputFile 
    # only one straw file is needed
    command += "mv -v %s straws.%d.txt ; " % (files_list_stw[0], runNumber)
    # copy also db const
    command += "mv -v dbconst.txt %s.dbconst.txt" % outputFile    
    
    tryError(command,"ERROR: Failed in process hadd\n")  

    ##################################################################################################
    nextstep("Make all plots")
    ##################################################################################################

    # generating the .ps file
    command  = "TRTCalib_makeplots itersum %s.merge.root %s/lastconfigfile" % (outputFile, os.path.abspath('.'))
    
    tryError(command,"ERROR: Failed in creating plots (itersum.ps file)\n")  

    ##################################################################################################
    nextstep("converting ps to pdf")
    ##################################################################################################
    
    command  = "ps2pdf itersum.ps %s.itersum.pdf" %(outputFile)
    
    tryError(command,"ERROR: Failed in creating itersum.pdf from itersum.ps)\n")
    
    
    
    
    
    # WORK IN PROGRESS! - FUTURE MR
    
    ##################################################################################################
    #
    #       Merging *.tracktuple.root files
    #
    ##################################################################################################
    
    # try:
    #     command = 'hadd -f %s.tracktuple.root %s' % (runArgs.outputTAR_MERGEDFile, "".join(("%s.tracktuple.root " % str(file)) for file in runArgs.inputTARFile ))
    #     print("\n Running: %s \n" % (command))
    #     stdout, stderr = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    #     print("OUTPUT:\n%s" % (stdout.decode('ascii')))
    #     print("ERRORS:\n%s" % ("NONE" if stderr.decode('ascii')=='' else stderr.decode('ascii')))        
    # except OSError as e:
    #     print("ERROR: Failed in process merging *.tracktuple.root files\n",e)
    #     sys.exit(e.errno)

    ##################################################################################################
    #
    #       Merging *.straw.txt files
    #
    ##################################################################################################
    
    # try:
    #     command = 'TRTCalib_StrawStatus_merge %s.merged.straw.txt %s' % (runArgs.outputTAR_MERGEDFile, "".join(("%s.straw.txt " % str(file)) for file in runArgs.inputTARFile ))
    #     print("\n Running: %s\n" % (command))
    #     stdout, stderr = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    #     print("OUTPUT:\n%s" % (stdout.decode('ascii')))
    #     print("ERRORS:\n%s" % ("NONE" if stderr.decode('ascii')=='' else stderr.decode('ascii')))
    # except OSError as e:
    #     print("ERROR: Failed in process merging *.straw.txt files\n",e)
    #     sys.exit(e.errno)
    
        
    ##################################################################################################
    #
    #       Compressing outputs in a tar file!
    #
    ################################################################################################## 
       
    # try:
    #     # Getting list of files to be compressed
    #     files_list=glob.glob(runArgs.outputTAR_MERGEDFile+".*")
    #     # Compressing
    #     tar = tarfile.open(runArgs.outputTAR_MERGEDFile, "w:gz")
    #     print("\nCompressing files in %s output file:" % runArgs.outputTAR_MERGEDFile)
    #     for file in files_list:
    #         print("\t-",file)
    #         tar.add(file)
    #     tar.close()
    # except OSError as e:
    #     print("ERROR: Failed compressing the output files\n",e)
    #     sys.exit(e.errno)        
            
    
    # Prints all types of txt files present in a Path
    # print("\nListing files:")
    # for file in sorted(glob.glob("./*", recursive=True)):
    #     print("\t-",file)    