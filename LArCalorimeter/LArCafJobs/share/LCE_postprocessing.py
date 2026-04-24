#!/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys,os,subprocess,tempfile
from LArCafJobs.GetLBsToIgnore import getLBsToIgnore

eoscmd="eos "

if __name__=="__main__": 
    if len(sys.argv)<2 or not sys.argv[1].isdigit():
        print("Usage: %s <runnumber> {outputfile}" % sys.argv[0])
        sys.exit(-1)
    

    runnumber=int(sys.argv[1])
    if len(sys.argv)>2:
        outputFile=sys.argv[2]
    else:
        outputFile="LCE_CellList_%i.txt" % runnumber

    if not os.access(os.path.dirname(outputFile),os.W_OK):
        print("ERROR, no write access to output text file",outputFile)
        sys.exit(-1)
        

    #1. Try find input file on eos
    path=None

    #Get project tag (from SFO DB):
    from GetProjectTag import GetProjectTag
    projectTag=GetProjectTag(runnumber)
    if projectTag is None:
        print("Failed to get ProjectTag for run ",runnumber," from SFO DB")
        sys.exit(-2)
    else:
        print("Found project tag ",projectTag," for run ",runnumber)

    eospath="/eos/atlas/atlascerngroupdisk/det-larg/Tier0/perm/%s/calibration_LArCellsEmpty/%8.8i/" % (projectTag,runnumber)
    cmd = eoscmd+" ls "+eospath
    print(cmd)
    sRes = subprocess.getstatusoutput(cmd)
    print(sRes[1])
    output = sRes[1].split('\n')
        
    found=False
    if (sRes[0] == 0):
        print("output: ", output)
        for d1 in output:
            if d1.find("NTUP_SAMPLESMON")!=-1:
                eospath+="/"+d1
                found=True
                break
            pass
        if not found:
            print("Directory for LCE output of run %i not found on eos" % runnumber)
            sys.exit(-1)
    
        cmd = eoscmd+" ls "+eospath
        print("Checking path ",eospath)
        sRes = subprocess.getstatusoutput(cmd)
        print(sRes[1])
        output = sRes[1].split('\n')
        if (sRes[0] == 0):
            for d2 in output:
                print(d2)
                if d2 is not None and d2.find("NTUP_SAMPLESMON")!=-1:
                    filename=d2
                    path="root://eosatlas/"+eospath+"/"+filename
                    break
                pass
            pass
        pass
    pass
    
    if path is not None:
        print("Found project Tag",projectTag)
        print("Input file ",path)
    else:
        print("LCE output of run %i not found on eos" % runnumber)
        sys.exit(-1)


    workdir=tempfile.mkdtemp()
    LCEFile=workdir+"/"+filename
    cmd=eoscmd+" cp "+path+" "+LCEFile
    sRes = subprocess.getstatusoutput(cmd) 
    if (sRes[0] != 0 or not os.access(LCEFile,os.R_OK)):
        print("ERROR, failed to copy LCE ntuple to local directory")
        sys.exit(-1)
        pass


    #2. Get LBs to ignore:
    requireStableBeam=projectTag.endswith("TeV")
    bulk=False
    badLBs=getLBsToIgnore(runnumber,True,bulk,requireStableBeam)
    (badLBsF,badLBsFN)=tempfile.mkstemp(text=True)
    #print(badLBsF,badLBsFN)
    os.write(badLBsF,str.encode(', '.join([ str(i) for i in sorted(badLBs) ])))
    os.write(badLBsF,str.encode("\n"))
    os.close(badLBsF)
    print(badLBsFN)


    #3. running the list creation

    print("Running LCE_CellList",LCEFile,outputFile,badLBsFN)
    
    sc=subprocess.call(["LCE_CellList",LCEFile,outputFile,badLBsFN])
    
    os.remove(badLBsFN)
    os.remove(LCEFile)
