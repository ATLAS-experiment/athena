#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    ReadLumiFromCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-12-12
#
# Purpose: Read lumi values from CALO*PileUpNoiseLumi* tags
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dump Lumi values from online or offline CALO DB or from sqlite file")
    print ("")
    print ("-h, --help      shows this help")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")
    print ("-f, --folder=   specify folder to use e.g. /CALO/Ofl/Noise/PileUpNoiseLumi")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or tag suffix like RUN2-UPD4-04")
    print ("-c, --channel=  specify COOL channel, by default COOL channels 0 and 1 are used")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")

letters = "hs:t:f:c:r:l:b:e:"
keywords = ["help","schema=","tag=","folder=","channel=","run=","lumi=","begin=","end="]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run    = 2147483647
lumi   = 0
schema = 'CREST'
#folderPath = '/CALO/Noise/PileUpNoiseLumi'
folderPath = '/CALO/Ofl/Noise/PileUpNoiseLumi'
dbName = 'CONDBR2'
tag    = 'UPD4'
begin  = 0
end = 2147483647
iov = False
channels = [0,1]

for o, a in opts:
    a = a.strip()
    if o in ("-s","--schema"):
        schema = a
    elif o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-c","--channel"):
        channels = [int(a)]
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-b","--begin"):
        begin = int(a)
        iov = True
    elif o in ("-e","--end"):
        end = int(a)
        iov = True
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        usage()
        sys.exit(2)

from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython.TileCalibLogger import getLogger

#=== get a logger
log = getLogger("ReadLumi")
import logging
log.setLevel(logging.DEBUG)

#=== Set tag and schema name:

#=== Initialize blob reader
folderTAG = tag.upper()
if folderTAG.startswith("CALO") :
    folderPath=""
if not os.path.isfile(schema):
    log.info("Initializing folder %s with tag %s", folderPath, tag)
blobReader = TileCalibCrest.TileBlobReaderCrest(schema, folderPath, tag, run, lumi, channels[0], channels[-1], True)

#=== Filling the iovList
iovList = []
if iov:
    iovList = blobReader.getIovs((begin,0),(end,0))
    be=iovList[0][0]
    en=iovList[-1][0]

    if begin != be or end != en:
        log.info( "" )
        if be != begin:
            log.info( "Changing begin run from %d to %d (start of IOV)", begin,be)
            begin=be
        if en != end:
            if en>end:
                log.info( "Changing end run from %d to %d (start of next IOV)", end,en)
            else:
                log.info( "Changing end run from %d to %d (start of last IOV)", end,en)
            end=en
        log.info( "%d IOVs in total", len(iovList) )
else:
    iovList.append((run,lumi))

log.info( "\n" )

#=== loop over all iovs
obj = None
pref = ""
pref1 = ""
suff = ""
for iovs in iovList:
    if iov:
        pref = "(%i,%i)  " % (iovs[0],iovs[1])
        pref1 = pref
    values = []
    try:
        obj = blobReader.getPayload(iovs, False)
        for chan in channels:
            values += [obj[str(chan)]]
            pref1 = pref+"lumi"
    except Exception:
        obj = None
        log.warning( "Warning: can not read data from input DB" )
    if not iov and obj is not None:
        io = blobReader.getIov()
        (sinceRun,sinceLum) = (io[0][0],io[0][1])
        (untilRun,untilLum) = (io[1][0],io[1][1])
        suff = " iov since [%d,%d] until (%d,%d)" % (sinceRun,sinceLum,untilRun,untilLum)
    if len(values)==1:
        print(pref1,values[0],suff)
    else:
        print(pref1,values,suff)
