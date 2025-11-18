#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Sanya Solodkov 2025-10-20
#
# Purpose: Read channel status from CREST DB or from JSON file
# ReadBchFromCrest.py  --schema='CREST' --tag='UPD4'
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps the TileCal status bits from various schemas / folders")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify status folder to use ONL01 or OFL02, don't need to specify full path")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or full suffix like RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")
    print ("-m, --module=   specify module to use, default is not set")
    print ("-N, --chmin=    specify minimal channel to use, default is 0")
    print ("-X, --chmax=    specify maximal channel to use, default is 47")
    print ("-c, --chan=     specify channel to use , default is all channels from chmin to chmax")
    print ("-g, --gain=, -a, --adc=  specify adc(gain) to use, default is 2 (i.e. both low and high gains)")
    print ("-C, --comment   print comment for every IOV")
    print ("-i, --iov       print IOVs only for every module")
    print ("-I, --IOV       print IOVs only")
    print ("-d, --default   print also default values stored in AUX01-AUX20")
    print ("-B, --blob      print additional blob info")
    print ("-H, --hex       print frag id instead of module name")
    print ("-P, --pmt       print pmt number in addition to channel number")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")
    #print ("-D, --dbname=   specify dbname part of schema if schema only contains file name, default is CONDBR2")
    #print ("-S, --server=   specify server - ORACLE or FRONTIER, default is FRONTIER")
    print ("-w, --warning   suppress warning messages about missing drawers in DB")

letters = "hr:l:s:t:f:D:S:dBHPwm:b:e:a:g:c:N:X:CiI"
keywords = ["help","run=","lumi=","schema=","tag=","folder=","dbname=","server=","default","blob","hex","pmt","warning","module=","begin=","end=","chmin=","chmax=","gain=","adc=","chan=","comment","iov","IOV"]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run = 2147483647
lumi = 0
#schema = "output.%s.json" % (run)
schema = 'CREST'
dbname = ''
server = ''
folderPath =  "/TILE/OFL02/STATUS/ADC"
tag = "UPD4"
rosmin = 1
rosmax = 5
blob = False
hexid = False
pmt = False
warn = 1
modmin = 0
modmax = 99999
modulename="AUX-1"
partname=""
one_mod = False
mod = -1
ros = -1
chan_n= -1
chanmin = -1
chanmax = -1
gain_n = -1
gainmin = -1
gainmax = -1
begin = -1
end = 2147483647
iov = False
iovonly = False
IOVONLY = False
comment = False

for o, a in opts:
    a = a.strip()
    if o in ("-f","--folder"):
        if '/TILE' in a:
            folderPath = a
        else:
            folderPath = "/TILE/%s/STATUS/ADC" % a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-s","--schema"):
        schema = a
    elif o in ("-D","--dbname"):
        dbname = a
    elif o in ("-S","--server"):
        server = a
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-b","--begin"):
        begin = int(a)
        iov = True
        one_mod = True
    elif o in ("-e","--end"):
        end = int(a)
        iov = True
        one_mod = True
    elif o in ("-i","--iov"):
        iov = True
        iovonly = True
        if modulename=='AUX-1':
            modulename='ALL00'
    elif o in ("-I","--IOV"):
        iov = True
        IOVONLY = True
        if modulename=='AUX-1':
            modulename='ALL00'
    elif o in ("-a","--adc","-g","--gain"):
        gain_n = int(a)
    elif o in ("-m","--module"):
        modulename = a
        one_mod = True
    elif o in ("-c","--chan"):
        chan_n = int(a)
    elif o in ("-N","--chmin"):
        chanmin = int(a)
    elif o in ("-X","--chmax"):
        chanmax = int(a)
    elif o in ("-C","--comment"):
        comment = True
    elif o in ("-d","--default"):
        rosmin = 0
    elif o in ("-B","--blob"):
        blob = True
    elif o in ("-H","--hex"):
        hexid = True
    elif o in ("-P","--pmt"):
        pmt = True
    elif o in ("-w","--warning"):
        warn = -1
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        raise RuntimeError("unhandled option")


from TileCalibBlobPython import TileBchCrest
from TileCalibBlobObjs.Classes import TileCalibUtils

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("ReadBchFrCrest")
import logging
logLevel=logging.DEBUG
log.setLevel(logLevel)
log1 = getLogger("TileBchCrest")
log1.setLevel(logLevel)
log2 = getLogger("TileCalibCrest")
log2.setLevel(logLevel)

if tag.upper().endswith('HEAD'):
    tag=tag.upper()
if len(tag)==0 or tag.endswith('HEAD'):
    folderPath='/TILE/ONL01/STATUS/ADC'
    log.info("tag is %s, using %s folder", tag if tag else 'empty', folderPath)
    if tag=='HEAD':
        tag=''

folderTag = tag
if folderTag.upper().startswith("TILE") or folderTag.upper().startswith("CALO") :
    folderPath=""
log.info("Initializing folder %s with tag %s", folderPath, folderTag)

#=== create bad channel manager
mgr = TileBchCrest.TileBchMgr()
mgr.setLogLvl(logLevel)
mgr.initialize(schema, folderPath, folderTag, (run,lumi), warn, -2)
if iov or comment or warn<0:
    blobReader = mgr.getBlobReader()

#=== Dump the current isBad definition
isBadDef = mgr.getAdcProblems(0, TileCalibUtils.definitions_draweridx(), TileCalibUtils.bad_definition_chan(), 0)
if len(list(isBadDef.keys())):
    log.info( "isBad Definition: " )
    for prbCode in sorted(isBadDef.keys()):
        prbDesc = isBadDef[prbCode]
        msg = "- %2i (%s)" % (prbCode,prbDesc)
        log.info( msg )
#=== Dump the current isBadTiming definition
isBadTimingDef = mgr.getAdcProblems(0, TileCalibUtils.definitions_draweridx(), TileCalibUtils.badtiming_definition_chan(), 0)
if len(list(isBadTimingDef.keys())):
    log.info( "isBadTiming Definition: " )
    for prbCode in sorted(isBadTimingDef.keys()):
        prbDesc = isBadTimingDef[prbCode]
        msg = "- %2i (%s)" % (prbCode,prbDesc)
        log.info( msg )


#=== check ROS and module numbers
if one_mod:
    partname = modulename[:3]
    mod = int(modulename[3:]) -1

part_dict = {'AUX':0,'LBA':1,'LBC':2,'EBA':3,'EBC':4}
if partname in part_dict:
    ros = part_dict[partname]
    rosmin = ros
    rosmax = ros+1
else:
    ros = -1

if mod >= 0:
    modmin = mod
    modmax = mod+1
elif mod < -1:
    modmax = modmin

if chan_n >= 0 and chan_n < TileCalibUtils.max_chan():
    chanmin = chan_n
    chanmax = chan_n+1
else:
    if chanmin<0:
        chanmin = 0
    if chanmax<0:
        chanmax = TileCalibUtils.max_chan()
    else:
        chanmax += 1

if gain_n >= 0 and gain_n < TileCalibUtils.max_gain():
    gainmin = gain_n
    gainmax = gain_n+1
else:
    gainmin = 0
    gainmax = TileCalibUtils.max_gain()


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
        ib=0
        ie=len(iovList)
        for i,iovs in enumerate(iovList):
            run = iovs[0]
            lumi = iovs[1]
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
        log.info( "%d IOVs in total", len(iovList))

        #=== IOV only option
        if iovonly or IOVONLY:
            alliovs={}
            allmods={}
            zeroiovs={}
            nmod=0
            for ros in range(rosmin,rosmax):
                for mod in range(modmin, min(modmax,TileCalibUtils.getMaxDrawer(ros))):
                    allmods[TileCalibUtils.getDrawerString(ros,mod)] = ""
                    nmod+=1
            for since in iovList:
                iov="(%s,%s)" % since
                allmod=""
                zeromod=0
                miss=0
                for ros in range(rosmin,rosmax):
                    for mod in range(modmin, min(modmax,TileCalibUtils.getMaxDrawer(ros))):
                        flt = blobReader.getDrawer(ros, mod, since, False, False)
                        if flt is not None:
                            mod = TileCalibUtils.getDrawerString(ros,mod)
                            if flt==0:
                                zeromod += 1
                                allmod += " " + mod + "_zero"
                                allmods[mod] += " " + iov + "_zero"
                            else:
                                allmod += " " + mod
                                allmods[mod] += " " + iov
                        else:
                            miss+=1
                if miss==0 and nmod>1:
                    alliovs[iov] = " All %d modules" % nmod
                else:
                    alliovs[iov] = allmod
                zeroiovs[iov] = zeromod
            print("")
            if len(iovList)>0:
                if iovonly:
                    for key,value in allmods.items():
                        if value=="":
                            value=" None"
                        print("%s\t%s" % (key,value))
                if IOVONLY:
                    for key,value in alliovs.items():
                        if value=="":
                            value=" None"
                        if zeroiovs[key] and zeroiovs[key]>0:
                            print("%s\t%s - zero-sized blobs for %d modules" % (key,value,zeroiovs[key]))
                        else:
                            print("%s\t%s" % (key,value))
            else:
                print("No IOVs found")
            sys.exit(0)
else:
    iovList.append((run,lumi))

log.info( "\n" )


##channel2pmt
##negative means not connected !
##
dummy = [0]*48
barrel = [ 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
          13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
          27, 26, 25, 30, 29, 28,-33,-32, 31, 36, 35, 34,
          39, 38, 37, 42, 41, 40, 45,-44, 43, 48, 47, 46]
extbar = [ 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
          13, 14, 15, 16, 17, 18,-19,-20, 21, 22, 23, 24,
         -27,-26,-25,-31,-32,-28, 33, 29, 30,-36,-35, 34,
          44, 38, 37, 43, 42, 41,-45,-39,-40,-48,-47,-46]

ch2pmt = [ dummy, barrel, barrel, extbar, extbar ]
gname = [ "LG", "HG" ]

#=== Get ADC problems

#=== get the channel status (if it is bad and will be masked)
#=== the channel status depends on the definition of isBad stored
#=== in the database drawer 1, channel 0
#=== isAffected = has a problem not included in isBad definition
#=== isGood = has no problem at all

pref = ""
for iovs in iovList:
    if iov:
        pref = "(%i,%i)  " % (iovs[0],iovs[1])
        mgr.updateFromDb(schema, folderPath, folderTag, iovs, 1, warn)
    if comment:
        log.info( pref + str(blobReader.getComment(iovs)) )
    modOk = False
    miss  = 0
    good  = 0
    aff   = 0
    bad   = 0
    nMod  = 0
    for ros in range(rosmin,rosmax):
        for mod in range(modmin, min(modmax,TileCalibUtils.getMaxDrawer(ros))):
            nMod += 1
            if hexid:
                modName = "0x%x" % ((ros<<8)+mod)
            else:
                modName = TileCalibUtils.getDrawerString(ros,mod)
            if warn<0:
                bch = blobReader.getDrawer(ros, mod, iovs, False, False)
                if bch is None:
                    modOk = False
                    miss+=1
                    #print ("%s is missing in DB" % modName)
                else:
                    modOk = True
                    if blob and bch:
                        print ("%s  Blob type: %d  Version: %d  Nchannels: %d  Ngains: %d  Nval: %d" % (modName, bch.getObjType(), bch.getObjVersion(), bch.getNChans(), bch.getNGains(), bch.getObjSizeUint32()))
            nBad=0
            for chn in range(chanmin,chanmax):
                chnName = " %2i" % chn
                for adc in range(gainmin,gainmax):

                    stat = mgr.getAdcStatus(ros,mod,chn,adc)
                    #log.info( "- ADC status = isBad:      %d" % stat.isBad()      )
                    #log.info( "- ADC status = isGood:     %d" % stat.isGood()     )
                    #log.info( "- ADC status = isAffected: %d" % stat.isAffected() )

                    #=== get all problems of the channel
                    prbs = mgr.getAdcProblems(ros,mod,chn,adc)
                    #log.info( "ADC Problems: " )
                    if len(prbs) or iov:
                        modOk = False
                        if pmt:
                            msg = "%s pm %02i ch %02i %s " % ( modName, abs(ch2pmt[ros][chn]), chn, gname[adc] )
                        else:
                            msg = "%s %2i %1i " % ( modName,chn,adc )
                        for prbCode in sorted(prbs.keys()):
                            prbDesc = prbs[prbCode]
                            msg += " %5i (%s)" % (prbCode,prbDesc)
                        if stat.isBad():
                            msg += "  => BAD"
                            nBad+=1
                        elif stat.isAffected():
                            msg += "  => Affected"
                        elif stat.isGood():
                            msg += "  => good"
                        print (pref+msg)
            if modOk:
                good+=1
                print ("%s ALL GOOD" % (modName))
            elif nBad==0:
                aff+=1
            elif nBad==TileCalibUtils.max_gain()*TileCalibUtils.max_chan() or (nBad==90 and 'LB' in modName) or (nBad==64 and 'EB'in modName) or (nBad==60 and 'EBA15' in modName) or (nBad==60 and 'EBC18' in modName):
                bad+=1
    if warn<0:
        if miss:
            print ("%3i drawers are missing in DB" % miss)
        print ("%3i drawers are absolutely good" % good)
        print ("%3i drawers have good and affected channels" % (aff-miss))
        print ("%3i drawers have some bad channels" % (nMod-good-bad-aff))
        print ("%3i drawers are completely bad" % bad)
    #=== print all bad channels
    #log.info("listing bad channels")
    #mgr.listBadAdcs()
