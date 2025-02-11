#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# WriteLumiToCool.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>
#
# Purpose: Manual update of lumi values in CALO COOL DB
#

import getopt,sys,os,re
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Prepare sqlite file with Lumi values for CALO database")
    print ("")
    print ("-h, --help      shows this help")
    print ("-i, --infile=   specify the input sqlite file for comparison or full schema string")
    print ("-o, --outfile=  specify the output sqlite file")
    print ("-t, --tag=      specify the tag")
    print ("-f, --folder=   specify folder to use e.g. /CALO/Ofl/Noise/PileUpNoiseLumi ")
    print ("-d, --dbname=   specify the database name e.g. OFLP200")
    print ("-S, --server=   specify server - ORACLE or FRONTIER, default is FRONTIER")
    print ("-v, --value=    specify new lumi value")
    print ("-V, --value2=   specify new valid flag")
    print ("-c, --channel=  specify COOL channel, by default COOL channels 0 and 1 are used")
    print ("-r, --run=      specify run number for start of IOV")
    print ("-l, --lumi=     specify lumiblock number for start of IOV, default is 0")
    print ("-u  --update    set this flag if output sqlite file should be updated, otherwise it'll be recreated")

letters = "hi:o:t:f:d:S:v:V:c:r:l:u"
keywords = ["help","infile=","outfile=","tag=","folder=","dbname=","server=","value=","value2=","channel=","run=","lumi=","update"]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
inFile      = 'COOLOFL_CALO/CONDBR2'
outFile     = 'caloSqlite.db'
tag         = ''
folderPath = ''
dbName      = 'CONDBR2'
server      = ''
value       = None
value2      = 0
run         = -1
lumi        = 0
update      = False
channels = [0,1]

for o, a in opts:
    a = a.strip()
    if o in ("-i","--infile"):
        inFile = a
    elif o in ("-o","--outfile"):
        outFile = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-f","--folder"):
        folderPath = a
    elif o in ("-d","--dbname"):
        dbName = a
    elif o in ("-S","--server"):
        server = a
    elif o in ("-c","--channel"):
        channels = [int(a)]
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-u","--update"):
        update = True
    elif o in ("-v","--value"):
        value = a
    elif o in ("-V","--value2"):
        value2 = a
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        print (o, a)
        usage()
        sys.exit(2)

#=== check presence of all parameters
print ("")
if len(inFile)<1:
    raise Exception("Please, provide infile (e.g. --infile=COOLOFL_TILE/OFLP200)")
if len(outFile)<1:
    raise Exception("Please, provide outfile (e.g. --outfile=caloSqlite.db)")
if len(tag)<1:
    raise Exception("Please, provide tag (e.g. --tag=RUN2-UPD4-04)")
if len(dbName)<1:
    raise Exception("Please, provide dbname (e.g. --dbname=OFLP200 or --dbname=CONDBR2)")
if value is None:
    raise Exception("Please, provide value (e.g. --value=12345)")
if run<0:
    raise Exception("Please, provide run number (e.g. --run=123456)")

from PyCool import cool
from CaloCondBlobAlgs import CaloCondTools, CaloCondLogger

#=== get a logger
log = CaloCondLogger.getLogger("WriteLumi")
import logging
log.setLevel(logging.DEBUG)

#=== Set tag and schema name:

if tag=="HEAD":
    tag=""

if os.path.isfile(inFile):
    ischema = 'sqlite://;schema='+inFile+';dbname='+dbName
else:
    log.info("File %s was not found, assuming it's full schema string" , inFile)
    ischema = inFile
    # possible strings for inFile - full schema connection string or short names like
    # COOLONL_CALO/CONDBR2 COOLOFL_CALO/CONDBR2 COOLOFL_CALO/OFLP200

#=== Open DB connections
oschema = 'sqlite://;schema='+outFile+';dbname='+dbName
dbr = CaloCondTools.openDbConn(ischema,server)
update = update or (inFile==outFile)
dbw = CaloCondTools.openDbConn(oschema,('UPDATE' if update else 'RECREATE'))

if len(folderPath)==0:
    if 'ONL' in ischema:
        folderPath = '/CALO/Noise/PileUpNoiseLumi'
    else:
        folderPath = '/CALO/Ofl/Noise/PileUpNoiseLumi'

if tag=='UPD1' or tag=='UPD4':
    from TileCalibBlobPython import TileCalibTools
    folderTag = TileCalibTools.getFolderTag(dbr, folderPath, tag )
elif folderPath.startswith('/CALO/Ofl/Noise/PileUpNoiseLumi'):
    folderTag = 'CALOOflNoisePileUpNoiseLumi-'+tag
elif folderPath.startswith('/CALO/Noise/PileUpNoiseLumi'):
    folderTag = 'CALONoisePileUpNoiseLumi-'+tag
else:
    folderTag = tag

#=== creating folder specifications
spec = cool.RecordSpecification()
spec.extend( 'LBAvInstLumi', cool.StorageType.Float )
spec.extend( 'Valid', cool.StorageType.UInt32 )

multiVersion=(len(folderTag)>0)

if multiVersion:
    folderMode = cool.FolderVersioning.MULTI_VERSION
    folderSpec = cool.FolderSpecification(folderMode, spec)
else:
    folderMode = cool.FolderVersioning.SINGLE_VERSION
    folderSpec = cool.FolderSpecification(folderMode, spec)

log.info( "Using folder %s with tag %s", folderPath,folderTag )
folderDescr = CaloCondTools.getAthenaFolderDescr()
folderR = dbr.getFolder(folderPath)
if update:
    try:
        folderW = dbw.getFolder(folderPath)
    except Exception:
        folderW = dbw.createFolder(folderPath, folderSpec, folderDescr, True)
else:
    folderW = dbw.createFolder(folderPath, folderSpec, folderDescr, True)

newval1 = float(value)
newval2 = int(value2)
newiov = "[%d,%d] - infinity" % (run, lumi)
since = CaloCondTools.iovFromRunLumi( run, lumi )
until = CaloCondTools.iovFromRunLumi( CaloCondTools.MAXRUN, CaloCondTools.MAXLBK )

for chan in channels:
    try:
        obj = folderR.findObject( since, chan, folderTag )
        (sinceRun,sinceLum) = CaloCondTools.runLumiFromIov(obj.since())
        (untilRun,untilLum) = CaloCondTools.runLumiFromIov(obj.until())
        val1 = obj.payload()[0]
        val2 = obj.payload()[1]
        oldiov = "[%d,%d] - (%d,%d)" % (sinceRun,sinceLum,untilRun,untilLum)
    except Exception:
        val1 = None
        val2 = None
        oldiov = "[NaN,NaN] - (NaN,NaN)"
        #log.warning("IOV [%d,%d] was not found in input DB", run, lumi )

    print("COOL channel",chan,"old iov",oldiov," old values [",val1,",",val2,
          "]  new values [",newval1,",",newval2,"] new iov",newiov)

    spec = folderW.payloadSpecification()
    data = cool.Record( spec )
    data['LBAvInstLumi'] = newval1
    data['Valid'] = newval2
    folderW.storeObject(since, until, data, chan, folderTag, multiVersion)

#=== Cleanup
dbw.closeDatabase()
dbr.closeDatabase()
