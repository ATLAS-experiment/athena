#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    CopyBlobFromCool.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-02-04
#
# Purpose: Read blobs from COOL and write them to JSON file
#

import getopt,sys,os,json
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Read TileCal blobs from COOL and convert them to JSON format for CREST")
    print ("")
    print ("-h, --help      shows this help")
    print ("-s, --schema=   specify schema to use, ONL or OFL for RUN1 or ONL2 or OFL2 for RUN2 or MC")
    print ("-S, --server=   specify server - ORACLE or FRONTIER, default is FRONTIER")
    print ("-d, --dbname=   specify the database name e.g. CONDBR2")
    print ("-f, --folder=   specify folder to use e.g. /TILE/OFL02/STATUS/ADC")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or tag suffix like RUN2-UPD4-04")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-c, --channel=  specify COOL channel, by default COOL channels 0-275 and 1000 are used")
    print ("-f, --full      if sqlite file doesn't contain everything, try to find missing COOL channels in Oracle DB")
    print ("-o, --output=   specify the prefix for output json file")

letters = "hS:s:d:t:f:r:l:c:o:f"
keywords = ["help","server=","schema=","dbname=","tag=","folder=","run=","lumi=","channel=","output=","full"]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run    = 2147483647
lumi   = 0
server = ''
schema = 'COOLOFL_TILE/CONDBR2'
folderPath = '/TILE/OFL02/CALIB/CIS/LIN'
dbName = 'CONDBR2'
tag    = 'UPD4'
channels = list(range(276)) + [1000]
output = ""
full = False

for o, a in opts:
    a = a.strip()
    if o in ("-s","--schema"):
        schema = a
    elif o in ("-S","--server"):
        server = a
    elif o in ("-d","--dbname"):
        dbName = a
    elif o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-c","--channel"):
        channels = [int(a)]
    elif o in ("-o","--output"):
        output = a
    elif o in ("-f","--full"):
        full = True
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        usage()
        sys.exit(2)

import base64
import cppyy
from PyCool import cool
from TileCalibBlobPython import TileCalibTools
from TileCalibBlobObjs.Classes import TileCalibDrawerCmt

Blob = cppyy.gbl.coral.Blob

def make_blob(string):
    b = Blob()
    b.write(string)
    b.seek(0)
    return b

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("CopyBlob")
import logging
logLevel=logging.DEBUG
log.setLevel(logLevel)

#=== Read from COOL server:

if os.path.isfile(schema):
    schema = 'sqlite://;schema='+schema+';dbname='+dbName
else:
    log.info("File %s was not found, assuming it's full schema string" , schema)

db = TileCalibTools.openDbConn(schema,server)
folderTag = TileCalibTools.getFolderTag(db if 'CONDBR2' in schema else schema, folderPath, tag)
log.info("Initializing folder %s with tag %s", folderPath, folderTag)

folder = db.getFolder(folderPath)

log.info( "\n" )

payloadSpec = cool.RecordSpecification()
payloadSpec.extend( 'TileCalibBlob', cool.StorageType.Blob64k )
folderMode = cool.FolderVersioning.MULTI_VERSION
folderSpec = cool.FolderSpecification(folderMode, payloadSpec)

#=== loop over all COOL channels
obj = None
suff = ""
since = (run<<32)+lumi
maxSince = 0
jdata={}
missingChannels = []
if full and 'sqlite' in schema:
    maxIter = 2
else:
    maxIter = 1
for iter in range(maxIter):
    if missingChannels:
        dbOra = TileCalibTools.openDbOracle(server, schema, folderPath)
        folder = dbOra.getFolder(folderPath)
        channels = missingChannels
        missingChannels = []
    for chan in channels:
      try:
        obj = folder.findObject( since, chan, folderTag )
        objsince = obj.since()
        objuntil = obj.until()
        (sinceRun,sinceLum) = (objsince>>32,objsince&0xFFFFFFFF)
        (untilRun,untilLum) = (objuntil>>32,objuntil&0xFFFFFFFF)
        if (objsince>maxSince):
            maxSince=objsince
        coralblob = obj.payload()[0]
        blob = coralblob.read()
        b64string = str(base64.b64encode(blob),'ascii')
        jdata[chan] = [b64string]
        if chan==1000:
            cmt = TileCalibDrawerCmt.getInstance(coralblob)
            fullcmt = cmt.getFullComment()
            log.info(fullcmt+"\n")
            # back conversion - just for test
            blob1 = base64.decodebytes(bytes(b64string,'ascii'))
            coralblob = make_blob(blob1)
            cmt1 = TileCalibDrawerCmt.getInstance(coralblob)
            fullcmt1 = cmt1.getFullComment()
            if(fullcmt!=fullcmt1): log.error(fullcmt1+"\n")
      except Exception:
        if iter==1:
            log.warning( "Warning: can not read COOL channel %d from Oracle" , chan)
        else:
            if maxIter==2:
                log.info( "Info: COOL channel %d is missing in input DB, it will be taken from Oracle" , chan)
            else:
                log.warning( "Warning: can not read COOL channel %d from input DB" , chan)
        missingChannels.append(chan)
    if len(missingChannels)==0:
        break
if iter>0:
    jdata = dict(sorted(jdata.items(), key=lambda item: int(item[0])))

if output=="":
    output = folderTag
if "." not in output:
    (sinceRun,sinceLumi) = (maxSince>>32,maxSince&0xFFFFFFFF)
    suff = "." + str(sinceRun) + "." + str(sinceLumi) + ".json"
    ofile = output + suff
else:
    ofile = output
with open(ofile, 'w') as the_file:
    json.dump(jdata,the_file)
    the_file.write('\n')

print("see file",ofile)

#=== close DB
db.closeDatabase()
