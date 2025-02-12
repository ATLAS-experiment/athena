#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# ReadLumiFromCool.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>
#
# Purpose: Read lumi values from CALO COOL DB
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dump Lumi values from online or offline CALO DB or from sqlite file")
    print ("")
    print ("-h, --help      shows this help")
    print ("-s, --schema=   specify schema to use, ONL or OFL for RUN1 or ONL2 or OFL2 for RUN2 or MC")
    print ("-S, --server=   specify server - ORACLE or FRONTIER, default is FRONTIER")
    print ("-d, --dbname=   specify the database name e.g. CONDBR2")
    print ("-f, --folder=   specify folder to use e.g. /CALO/Ofl/Noise/PileUpNoiseLumi")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or tag suffix like RUN2-UPD4-04")
    print ("-c, --channel=  specify COOL channel, by default COOL channels 0 and 1 are used")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")

letters = "hS:s:d:t:f:c:r:l:b:e:"
keywords = ["help","server=","schema=","dbname=","tag=","folder=","channel=","run=","lumi=","begin=","end="]

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
schema = 'OFL2'
folderPath = ''
dbName = 'CONDBR2'
tag    = 'UPD4'
begin  = -1
end = 2147483647
iov = False
channels = [0,1]

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

from CaloCondBlobAlgs import CaloCondTools, CaloCondLogger

#=== get a logger
log = CaloCondLogger.getLogger("ReadLumi")
import logging
log.setLevel(logging.DEBUG)

#=== Set tag and schema name:

if schema=='ONL': # shortcut for COOLONL_CALO/COMP200
    schema='COOLONL_CALO/COMP200'
    if tag=='UPD1' or tag=='UPD4':
        tag='UPD1-00' # change default to latest RUN1 tag
elif schema=='ONL2': # shortcut for COOLONL_CALO/CONDBR2
    schema='COOLONL_CALO/CONDBR2'
    if tag=='UPD1' or tag=='UPD4':
        tag='RUN2-UPD1-00' # change default to the only tag available online
elif schema=='OFL': # shortcut for COOLOFL_CALO/COMP200
    schema='COOLOFL_CALO/COMP200'
    if tag=='UPD1':
        tag='UPD1-00' # change default to latest RUN1 tag
    if tag=='UPD4':
        tag='UPD4-02' # change default to latest RUN1 tag
elif schema=='OFL2': # shortcut for COOLOFL_CALO/CONDBR2
    schema='COOLOFL_CALO/CONDBR2'
elif schema=='ONLMC': # shortcut for COOLONL_CALO/OFLP200
    schema='COOLONL_CALO/OFLP200'
    if tag=='UPD4':
        tag='OFLCOND-RUN12-SDR-07' # change default to latest RUN1-RUN2 tag
elif schema=='OFLMC' or schema=='MC': # shortcut for COOLOFL_CALO/OFLP200
    schema='COOLOFL_CALO/OFLP200'
    if tag=='UPD4':
        tag='OFLCOND-MC23-SDR-RUN3-02' # change default to tag used in MC23

if tag=="HEAD":
    tag=""

if os.path.isfile(schema):
    schema = 'sqlite://;schema='+schema+';dbname='+dbName
else:
    log.info("File %s was not found, assuming it's full schema string" , schema)

#=== Open DB connections
db = CaloCondTools.openDbConn(schema, server)

if len(folderPath)==0:
    if 'ONL' in schema:
        folderPath = '/CALO/Noise/PileUpNoiseLumi'
    else:
        folderPath = '/CALO/Ofl/Noise/PileUpNoiseLumi'

if tag=='UPD1' or tag=='UPD4' or 'COND'in tag:
    from TileCalibBlobPython import TileCalibTools
    folderTag = TileCalibTools.getFolderTag(schema if 'COMP200' in schema or 'OFLP200' in schema else db, folderPath, tag )
elif folderPath.startswith('/CALO/Ofl/Noise/PileUpNoiseLumi'):
    folderTag = 'CALOOflNoisePileUpNoiseLumi-'+tag
elif folderPath.startswith('/CALO/Noise/PileUpNoiseLumi'):
    folderTag = 'CALONoisePileUpNoiseLumi-'+tag
else:
    folderTag = tag

log.info("Initializing folder %s with tag %s", folderPath, folderTag)

folder = db.getFolder(folderPath)

#=== Filling the iovList
iovList = []
if iov:
    try:
        blobReader = CaloCondTools.CaloBlobReader(db,folderPath, folderTag)
        dbobjs = blobReader.getDBobjsWithinRange(0)
        if (dbobjs is None):
            raise Exception("No DB objects retrieved when building IOV list!")
        while dbobjs.goToNext():
            obj = dbobjs.currentRef()
            since = CaloCondTools.runLumiFromIov(obj.since())
            until = CaloCondTools.runLumiFromIov(obj.until())
            iovList.append((since, until))
    except Exception:
        log.warning( "Warning: can not read IOVs from input DB file" )
        sys.exit(2)

    be=iovList[0][0][0]
    en=iovList[-1][0][0]

    if begin != be or end != en:
        ib=0
        ie=len(iovList)
        for i,iovs in enumerate(iovList):
            run = iovs[0][0]
            lumi = iovs[0][1]
            if (run<begin and run>be) or (run==begin and lumi==0) :
                be=run
                ib=i
            if run>=end and run<en:
                en=run
                ie=i+1
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
        iovList=iovList[ib:ie]
        log.info( "%d IOVs in total", len(iovList) )
else:
    iovList.append(((run,lumi),(CaloCondTools.MAXRUN, CaloCondTools.MAXLBK)))

log.info( "\n" )

#=== loop over all iovs and COOL channels
obj = None
pref = ""
pref1 = ""
suff = ""
for iovs in iovList:
    if iov:
        pref = "(%i,%i)  " % (iovs[0][0],iovs[0][1])
        pref1 = pref
    since = CaloCondTools.iovFromRunLumi(iovs[0][0],iovs[0][1])
    values = []
    for chan in channels:
        try:
            obj = folder.findObject( since, chan, folderTag )
            try:
                values += [[obj.payload()[0],obj.payload()[1]]]
                pref1 = pref+"lumi"
            except Exception:
                #log.warning( "Warning: can not interpert data as Lumi values" )
                pref1 = pref
                values += [obj.payload()[0]]
        except Exception:
            obj = None
            log.warning( "Warning: can not read data from input DB" )
    if not iov and obj is not None:
        (sinceRun,sinceLum) = CaloCondTools.runLumiFromIov(obj.since())
        (untilRun,untilLum) = CaloCondTools.runLumiFromIov(obj.until())
        suff = " iov since [%d,%d] until (%d,%d)" % (sinceRun,sinceLum,untilRun,untilLum)
    if len(values)==1:
        print(pref1,values[0],suff)
    else:
        print(pref1,values,suff)

#=== close DB
db.closeDatabase()
