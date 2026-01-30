#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    ReadCellNoiseCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-11-16
#
# Purpose: Read cell noise parameters from CREST DB or from JSON file
#
# Each Tile cell has 5 values stored in DB.
# The first two values are the RMS of a sigle gaussian model of the electronic noise
# and the pile-up noise normalized at 10^33cm-2s-1, (backwards compatibility)
# The next three values are used for a two gaussian model.
# These are: ratio between first and second gaussian, RMS of the first gaussian, and RMS of the second gaussian
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps noise constants from online or offline folders / tags")
    print ("")
    print ("-h, --help      shows this help")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")
    print ("-f, --folder=   specify status folder to use f.i. /TILE/OFL02/NOISE/CELL or /CALO/Noise/CellNoise")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or tag suffix like 14TeV-N200_dT50-01")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")
    print ("-C, --comment   print comment for every IOV")
    print ("-i, --iov       print IOVs only for every COOL channel")
    print ("-I, --IOV       print IOVs only")
    print ("-n, --channel=  specify COOL channel to read (48 by defalt)")
    print ("-c, --cell=     specify cell hash (0-5183), default is -1, means all cells")
    print ("-g, --gain=     specify gain to print (0-3), default is -1, means all gains")
    print ("-x, --index=    specify parameter index (0-4), default is -1, means all parameters")
    print ("-B, --brief     print only numbers without character names")
    print ("-D, --double    print values with double precision")

letters = "hs:t:f:r:l:b:e:n:c:g:x:BDCiI"
keywords = ["help","schema=","tag=","folder=","run=","lumi=","begin=","end=","channel=","cell=","gain=","index=","brief","double","comment","iov","IOV"]

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
folderPath = '/TILE/OFL02/NOISE/CELL'
tag    = 'UPD4'
chan   = 48 # represents Tile
cell   = -1
gain   = -1
index  = -1
brief  = False
doubl  = False
begin = -1
end = 2147483647
iov = False
iovonly = False
IOVONLY = False
comment = False

for o, a in opts:
    a = a.strip()
    if o in ("-s","--schema"):
        schema = a
    elif o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-n","--channel"):
        chan = int(a)
    elif o in ("-c","--cell"):
        cell = int(a)
    elif o in ("-g","--gain"):
        gain = int(a)
    elif o in ("-x","--index"):
        index = int(a)
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
    elif o in ("-i","--iov"):
        iov = True
        iovonly = True
    elif o in ("-I","--IOV"):
        iov = True
        IOVONLY = True
    elif o in ("-C","--comment"):
        comment = True
    elif o in ("-B","--brief"):
        brief = True
    elif o in ("-D","--double"):
        doubl = True
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        usage()
        sys.exit(2)

tile=(chan==48)

from TileCalibBlobPython import TileCalibLogger
from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython import TileCellTools

#=== get a logger
log = TileCalibLogger.getLogger("ReadCellNoise")

if run>=400000:
    cabling = 'RUN3'
elif run>=342550:
    cabling = 'RUN2a'
elif run>=222222:
    cabling = 'RUN2'
else:
    cabling = 'RUN1'

hashMgr=None
hashMgrDef=TileCellTools.TileCellHashMgr(cabling=cabling)
hashMgrA=TileCellTools.TileCellHashMgr("UpgradeA")
hashMgrBC=TileCellTools.TileCellHashMgr("UpgradeBC")
hashMgrABC=TileCellTools.TileCellHashMgr("UpgradeABC")

#=== Initialize blob reader
folderTAG = tag.upper()
if folderTAG.startswith("TILE") or folderTAG.startswith("CALO") or folderTAG.startswith("LAR") :
    folderPath=""
if not os.path.isfile(schema):
    log.info("Initializing folder %s with tag %s", folderPath, tag)
blobReader = TileCalibCrest.TileBlobReaderCrest(schema, folderPath, tag, run, lumi)
log.info("Comment: %s", blobReader.getComment())

#=== create CaloCondBlobFlt
blobFlt = blobReader.getDrawer(-1,chan,None,False,False)
if blobFlt is None:
    log.critical("Could not locate a data blob in CREST payload for COOL channel %s", chan)
    sys.exit(1)

#=== retrieve data from the blob
#cell  = 0 # 0..5183 - Tile hash
#gain  = 0 # 0..3    - four Tile cell gains: -11, -12, -15, -16
#index = 0 # 0..4    - electronic or pile-up noise or 2-G noise parameters

ncell=blobFlt.getNChans()
ngain=blobFlt.getNGains()
nval=blobFlt.getObjSizeUint32()

if ncell>hashMgrA.getHashMax():
    hashMgr=hashMgrABC
elif ncell>hashMgrBC.getHashMax():
    hashMgr=hashMgrA
elif ncell>hashMgrDef.getHashMax():
    hashMgr=hashMgrBC
else:
    hashMgr=hashMgrDef
log.info("Using %s CellMgr with hashMax %d", hashMgr.getGeometry(),hashMgr.getHashMax())

#=== Filling the iovList
iovList = []
if iov:
    if begin<0:
        if iovonly or IOVONLY:
            begin = end
        else:
            begin=0
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
        log.info( "%d IOVs in total", len(iovList))

        #=== IOV only option
        if iovonly or IOVONLY:
            option = 1 if iovonly else 0
            option += (2 if IOVONLY else 0)
            blobReader.dumpIovs(iovList,-1,0,chan,chan+1,option,tile,False)
            sys.exit(0)
else:
    iovList.append((run,lumi))

log.info( "\n" )


if cell<0 or cell>=ncell:
    cellmin=0
    cellmax=ncell
else:
    cellmin=cell
    cellmax=cell+1

if gain<0 or gain>=ngain:
    gainmin=0
    gainmax=ngain
else:
    gainmin=gain
    gainmax=gain+1

if index<0 or index>=nval:
    indexmin=0
    indexmax=nval
else:
    indexmin=index
    indexmax=index+1

if brief or doubl:
    name1 = ["","","0.0     "]
    names = []
    dm=" "
    for i in range(indexmax):
        names += [""]
else:
    name1 = ["Noise cell ", "gain ","0.00    "]
    names = ["RMS ", "pileup ", "RMS1 ", "RMS2 ", "Ratio "]
    for i in range(len(names),indexmax):
        names += ["c"+str(i)+" "]
    dm="\t"

oldBlob = None
pref = ""
for iovs in iovList:
    if iov:
        pref = "(%i,%i)  " % (iovs[0],iovs[1])
        newBlob = blobReader.getBlob(-1, chan, iovs, False)
        if oldBlob == newBlob:
            log.info( f'Cell noise in IOV {iovs} is identical to previous IOV')
            if comment:
                log.info( pref + str(blobReader.getComment(iovs)) )
            continue
        else:
            if oldBlob is None:
                log.info( f'Reading IOV {iovs}')
            else:
                log.info( f'Cell noise was updated in IOV {iovs}')
        oldBlob = newBlob
        blobFlt = blobReader.getDrawer(-1,chan,iovs,False,False)
    if comment:
        log.info( pref + str(blobReader.getComment(iovs)) )
    for cell in range(cellmin,cellmax):
        if tile and len(name1[0]):
            name1[0] = "%s %6s hash " % hashMgr.getNames(cell)
        for gain in range(gainmin,gainmax):
            msg="%s%4d %s%d\t" % ( name1[0], cell, name1[1], gain)
            for index in range(indexmin,indexmax):
                v=blobFlt.getData(cell, gain, index)
                if doubl:
                    msg += "%s%s%s" % (names[index],"{0:<15.10g}".format(v).ljust(15),dm)
                elif v<5.e-7:
                    msg += "%s%s%s" % (names[index],name1[2],dm)
                elif v<1:
                    msg += "%s%8.6f%s" % (names[index],v,dm)
                else:
                    msg += "%s%s%s" % (names[index],"{0:<8.7g}".format(v).ljust(8),dm)
            print (pref+msg)

