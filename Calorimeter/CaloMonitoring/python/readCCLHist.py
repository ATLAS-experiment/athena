#!/bin/env python 
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ROOT import TFile
from operator import itemgetter
import re
import os,sys


def readInput(filename, nCellsList=20, bcfile=None, cutoff=0.1):

    #Set up LArOnlineID helper class in standalone mode (from xml file)
    from ROOT import IdDictParser
    parser=IdDictParser()
    #Get xml files:
    xmlpath=None
    for dd in os.getenv('XMLPATH').split(os.pathsep):
        d=dd+"/IdDictParser/ATLAS_IDS.xml"
        if os.access(d,os.R_OK):
            xmlpath=dd
            break
    if not xmlpath:
        print ("ERROR, unable to locate identifier dictionaries")
        sys.exit(-1)
               

    parser.register_external_entity("LArCalorimeter",xmlpath+"/IdDictParser/IdDictLArCalorimeter_DC3-05-Comm-01.xml")
    idd = parser.parse(xmlpath+"/IdDictParser/ATLAS_IDS.xml")
    from ROOT import LArOnlineID
    larID=LArOnlineID()
    stat=larID.initialize_from_dictionary(idd)
    if stat==1:
        print ("ERROR, failed to init LArOnlineID")
        sys.exit(-1)


    #Open ROOT HIST file
    f=TFile.Open(filename)
    if not f.IsOpen():
        print("ERROR, failed to open input file",filename)
        sys.exit(-1)

    rundir=None
    for d in f.GetListOfKeys():
        mg=re.match("run_([0-9]*)",d.GetName())
        rundir=mg.group(0)
        break

    print("Found run dir:",rundir)
    runnbr=rundir[4:]

    hist=f.Get(rundir+"/CaloMonitoring/CaloClusterCellMon/Summary/larhashPercent")
    
    freq=[]
    for idx in range(hist.GetNbinsX()):
        freq.append((idx,hist.GetBinContent(idx)))
    
    freq.sort(key=itemgetter(1),reverse=True)
  
    for f in freq[:nCellsList]:
        c=larID.channel_Id(f[0])
        print ("Channel %s constributes to clusters in %.3f %% of events"% (larID.channel_name(c),f[1]))

  
    if bcfile:
        bcfile.write("#Bad channel list for run "+runnbr+"\n")
        for (h,f) in freq:
            if f<cutoff: break
            c=larID.channel_Id(h)
            bcfile.write("%i %i %i %i %i 0 highNoiseHG\n"% (larID.barrel_ec(c), larID.pos_neg(c), larID.feedthrough(c),
                                                                  larID.slot(c), larID.channel(c))) 

        bcfile.close()



if __name__=="__main__":
    
    import argparse
    parser= argparse.ArgumentParser()
    parser.add_argument("inputfile",type=argparse.FileType('r'),help="Input HIST file containig <run>/CaloMonitoring/CaloClusterCellMon/Summary/cellhashPercent")
    parser.add_argument('BCfile', type=argparse.FileType('w'),nargs='?',default=None,help="Optional output file digestable as bad-channel input")
    parser.add_argument("--cut",type=float,default=0.1,help="Write channels appearing more often that x %% as highNoiseHG")
    parser.add_argument("--nPrint",type=int,default=20,help="Print a list of the N most noisy channels")

    (args,leftover)=parser.parse_known_args()

    args.inputfile.close() #Care only about the name, will be opend by ROOT

    readInput(args.inputfile.name,args.nPrint,args.BCfile,args.cut)
