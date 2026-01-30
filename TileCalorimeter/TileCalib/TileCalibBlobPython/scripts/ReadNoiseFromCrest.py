#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    ReadNoiseFromCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-12-12
#
# Purpose: Read sample noise and non-iterative Opt.Filter noise values for single channel
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps the TileCal noise from SAMPLE and OFNI folders")
    print ("")
    print ("-h, --help      shows this help")
    print ("-t, --tag=      specify tag to use, f.i. RUN2-HLT-UPD1-01 or COM-01")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-p, --ros=      specify partition (ros number), default is 1")
    print ("-d, --drawer=   specify drawer number, default is 0")
    print ("-m, --module=   specify module to use, default is LBA01")
    print ("-c, --chan=     specify channel number, default is 0")
    print ("-g, -a, --adc=  specify gain (adc number), default is 0")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")

letters = "hr:l:s:t:p:d:m:c:a:g:"
keywords = ["help","run=","lumi=","schema=","tag=","ros=","drawer=","module","chan=","channel=","adc=","gain="]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run = 2147483647
lumi = 0
schema = 'CREST'
tag    = 'HEAD'
ros     = 1
drawer  = 0
channel = 0
adc     = 0

for o, a in opts:
    a = a.strip()
    if o in ("-t","--tag"):
        tag = a
    elif o in ("-s","--schema"):
        schema = a
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-m","--module"):
        partname = a[:3]
        part_dict = {'AUX':0,'LBA':1,'LBC':2,'EBA':3,'EBC':4}
        if partname in part_dict:
            ros = part_dict[partname]
            drawer = max(int(a[3:])-1,0)
    elif o in ("-p","--ros"):
        ros = int(a)
    elif o in ("-d","--drawer"):
        drawer = int(a)
    elif o in ("-c","--chan","--channel"):
        channel = int(a)
    elif o in ("-a","--adc","-g","--gain"):
        adc = int(a)
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        raise RuntimeError("unhandled option")



from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobObjs.Classes import TileCalibUtils

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("readNoise")
import logging
log.setLevel(logging.DEBUG)


folder1="/TILE/ONL01/NOISE/SAMPLE"
folder2="/TILE/ONL01/NOISE/OFNI"
log.info("Initializing ros %d, drawer %d for run %d, lumiblock %d", ros,drawer,run,lumi)
for folderPath in [folder1, folder2]:

    log.info("Initializing folder %s with tag %s", folderPath, tag)
    blobReader = TileCalibCrest.TileBlobReaderCrest(schema, folderPath, tag, run, lumi)
    log.info("... %s", blobReader.getComment((run,lumi)))
    blob = blobReader.getDrawer(ros, drawer,(run,lumi))

    if folderPath.find("SAMPLE")!=-1:
        ped = blob.getData(channel, adc, 0)
        hfn = blob.getData(channel, adc, 1)
        lfn = blob.getData(channel, adc, 2)
    else:
        rms = blob.getData(channel, adc, 0)
        plp = blob.getData(channel, adc, 1)

log.info( "\n" )
print ( "%s ch %i gn %i :  PED = %f  HFN = %f  LFN = %f    OF_RMS = %f  PILEUP = %f" %
        ( TileCalibUtils.getDrawerString(ros,drawer), channel, adc,
          ped, hfn, lfn, rms, plp) )
