#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    ReadOfcFromCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-12-12
#
# Purpose: Read Opt.Filter weights for single channel (all phases)
#


import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps the TileCal OFC values various schemas / folders / tags")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify status folder to use CIS, LAS, PHY ")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or full suffix like RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-p, --ros=      specify partition (ros number), default is 1")
    print ("-d, --drawer=   specify drawer number, default is 0")
    print ("-m, --module=   specify module to use, default is LBA01")
    print ("-c, --chan=     specify channel number, default is 0")
    print ("-g, -a, --adc=  specify gain (adc number), default is 0")
    print ("-i, --field=    specify field number, default is 0")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")

letters = "hr:l:s:t:f:p:d:m:c:a:g:i:"
keywords = ["help","run=","lumi=","schema=","tag=","folder=","ros=","drawer=","module=","chan=","channel=","adc=","gain=","field="]

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
folderPath =  "/TILE/ONL01/FILTER/OF2/PHY"
tag = ""
ros     = 1
drawer  = 0
channel = 0
adc     = 0
field   = 0

for o, a in opts:
    a = a.strip()
    if o in ("-f","--folder"):
        if a.startswith("/TILE"):
            folderPath = a
        elif a.startswith("OF"):
            folderPath = "/TILE/ONL01/FILTER/%s" % a
        else:
            folderPath = "/TILE/ONL01/FILTER/OF2/%s" % a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-s","--schema"):
        schema = a
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
    elif o in ("-i","--field"):
        field = int(a)
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        raise RuntimeError("unhandled option")


from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobObjs.Classes import TileCalibUtils

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("ReadOfc")
import logging
log.setLevel(logging.DEBUG)


if tag.upper().endswith('HEAD'):
    tag=tag.upper()
if len(tag)==0 or tag.endswith('HEAD'):
    folderPath=folderPath.replace('OFL02','ONL01')
    log.info("tag is %s, using %s folder", tag if tag else 'empty', folderPath)
    if tag=='HEAD':
        tag=''

folderTag = tag
if folderTag.upper().startswith("TILE") :
    folderPath=""
log.info("Initializing folder %s with tag %s", folderPath, folderTag)

blobReader = TileCalibCrest.TileBlobReaderCrest(schema,folderPath, folderTag, run, lumi)
#blobReader.log().setLevel(logging.DEBUG)

#=== get drawer with status at given run
log.info("Initializing ros %d, drawer %d for run %d, lumiblock %d", ros,drawer,run,lumi)
log.info("... %s", blobReader.getComment((run,lumi)))
ofc = blobReader.getDrawer(ros, drawer,(run,lumi))

#=== get other OFC parameters
nchann   = ofc.getNChans()
nfields  = ofc.getNFields()
nsamples = ofc.getNSamples()
nphases  = ofc.getNPhases()
log.info( "" )
log.info( "nchann   = %d", nchann  )
log.info( "nfields  = %d", nfields )
log.info( "nphases  = %d", nphases )
log.info( "nsamples = %d", nsamples)
log.info( "" )
log.info( "OFC for %s channel %d adc %d field %d\n", TileCalibUtils.getDrawerString(ros,drawer),channel,adc,field)

#=== get OFC field for given adc, sample and phase
for iphase in range(abs(nphases)):
    phase = ofc.getPhase(channel, adc, iphase)
    msg = "phase %6.1f ns :" % phase
    for smp in range(0, nsamples):
        msg += " %f" % ofc.getOfc(field, channel, adc, phase, smp)
    print (msg)
