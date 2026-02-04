#!/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# File:    ReadCalibFromCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-02-04
#
# Purpose: Read calibration constants from CREST DB or from JSON file
#

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps the TileCal constants from various schemas / folders / tags")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify status folder to use f.i. /TILE/OFL02/CALIB/CIS/LIN ")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or full suffix like RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")
    print ("-m, --module=   specify module to use, default is not set")
    print ("-N, --chmin=    specify minimal channel to use, default is 0")
    print ("-X, --chmax=    specify maximal channel to use, default is 47")
    print ("-c, --chan=     specify channel to use , default is all channels from chmin to chmax")
    print ("-g, --gain=, -a, --adc=  specify adc(gain) to print or number of adcs to print with - sign, default is -2")
    print ("-n, --nval=     specify number of values to output, default is all")
    print ("-C, --comment   print comment for every IOV")
    print ("-i, --iov       print IOVs only for every module")
    print ("-I, --IOV       print IOVs only")
    print ("-d, --default   print also default values stored in AUX01-AUX20")
    print ("-B, --blob      print additional blob info")
    print ("-H, --hex       print frag id instead of module name")
    print ("-P, --pmt       print pmt number in addition to channel number")
    print ("-p, --prefix=   print some prefix on every line ")
    print ("-k, --keep=     field numbers or channel numbers to ignore, e.g. '0,2,3,EBch0,EBch1,EBch12,EBch13,EBspD4ch18,EBspD4ch19,EBspC10ch4,EBspC10ch5' ")
    print ("-o, --double    print values with double precision")
    print ("-s, --schema=   specify name of input JSON file or CREST_SERVER_PATH")

letters = "hr:l:s:t:f:n:b:e:m:N:X:c:a:g:p:dBCiIHPk:o:"
keywords = ["help","run=","lumi=","schema=","tag=","folder=","module=","begin=","end=","chmin=","chmax=","gain=","adc=","chan=","nval=","prefix=","default","blob","hex","pmt","keep=","comment","iov","IOV","double"]

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
folderPath =  "/TILE/OFL02/CALIB/CIS/LIN"
tag = "UPD4"
nval = 0
nadc = -1
rosmin = 1
rosmax = 5
blob = False
hexid = False
pmt = False
prefix = None
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
gainmin = -1
gainmax = -1
begin = -1
end = 2147483647
iov = False
iovonly = False
IOVONLY = False
comment = False
keep=[]
doubl  = False

for o, a in opts:
    a = a.strip()
    if o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-s","--schema"):
        schema = a
    elif o in ("-n","--nval"):
        nval = int(a)
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
        nadc = int(a)
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
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-d","--default"):
        rosmin = 0
    elif o in ("-B","--blob"):
        blob = True
    elif o in ("-H","--hex"):
        hexid = True
    elif o in ("-P","--pmt"):
        pmt = True
    elif o in ("-p","--prefix"):
        prefix = a
    elif o in ("-k","--keep"):
        keep = a.split(",")
    elif o in ("-o","--double"):
        doubl = True
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        raise RuntimeError("unhandled option")


from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobObjs.Classes import TileCalibUtils, TileCalibType

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("ReadCalibFrCrest")
import logging
logLevel=logging.DEBUG
log.setLevel(logLevel)
log1 = getLogger("TileCalibCrest")
log1.setLevel(logLevel)

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

if iov:
    run=end
    lumi=0

if tag.upper().endswith('HEAD'):
    tag=tag.upper()
if len(tag)==0 or tag.endswith('HEAD'):
    folderPath=folderPath.replace('OFL02','ONL01')
    log.info("tag is %s, using %s folder", tag if tag else 'empty', folderPath)
    if tag=='HEAD':
        tag=''

folderTag = tag
if folderTag.upper().startswith("TILE") or folderTag.upper().startswith("CALO") :
    folderPath=""
log.info("Initializing folder %s with tag %s", folderPath, folderTag)

blobReader = TileCalibCrest.TileBlobReaderCrest(schema,folderPath, folderTag, run, lumi,
    TileCalibUtils.getDrawerIdx(max(rosmin,0),max(modmin,0)),
    TileCalibUtils.getDrawerIdx(min(rosmax-1,4),max(0,min(modmax-1,TileCalibUtils.getMaxDrawer(min(rosmax-1,4))-1))))

#=== get drawer with status at given run
flt=None
r=5
d=0
nchan=TileCalibUtils.max_chan()
ngain=TileCalibUtils.max_gain()
while not flt:
    d-=1
    if d<0:
        r-=1
        if r<0:
            break
        d=TileCalibUtils.getMaxDrawer(r)-1
    flt = blobReader.getDrawer(r, d, (run,lumi), True, False)
if flt:
    blobT=flt.getObjType()
    blobV=flt.getObjVersion()
    mchan=flt.getNChans()
    mgain=flt.getNGains()
    mval=flt.getObjSizeUint32()
    log.info( "Blob type: %d  Version: %d  Nchannels: %d  Ngains: %d  Nval: %d", blobT,blobV,mchan,mgain,mval)
    if nadc<-mgain:
        nadc=-mgain
    if nchan<mchan:
        nchan=mchan
    if ngain<mgain:
        ngain=mgain
else:
    mgain=1
if nadc==-1:
    nadc=-ngain

log.info("Comment: %s", blobReader.getComment((run,lumi)))

if chan_n >= 0 and chan_n < nchan:
    chanmin = chan_n
    chanmax = chan_n+1
else:
    if chanmin<0:
        chanmin = 0
    if chanmax<0:
        chanmax = nchan
    else:
        chanmax += 1

if nadc >= 0 and nadc < ngain:
    gainmin = nadc
    gainmax = nadc+1
else:
    gainmin = 0
    if nadc<0:
        gainmax = -nadc
    else:
        gainmax = ngain


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
            blobReader.dumpIovs(iovList,rosmin,rosmax,modmin,modmax,option,(rosmin<=0),True)
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
gname=[]
if mgain!=2:
    for i in range(mgain+1):
        gname+=[ "g "+str(i) ]
else:
    gname = [ "LG", "HG" ]

#=== loop over all partitions,modules,channels
oneModule = ((rosmax-rosmin==1) and (modmax-modmin==1) and iov)
oldBlob = None
pref = ""
for iovs in iovList:
    if iov:
        pref = "(%i,%i)  " % (iovs[0],iovs[1])
    if prefix:
        pref = prefix + " " + pref
    if comment:
        log.info( pref + str(blobReader.getComment(iovs)) )
    if prefix and prefix.startswith("Write"):
        comm = blobReader.getComment(iovs)
        if ": " in comm:
            comm = comm[comm.find(": ")+2:]
        print ('%s --update --folder=%s --tag=%s --run=%i --lumi=%i --comment="%s"' % (prefix,folderPath,folderTag,iovs[0],iovs[1],comm))
    miss=0
    good=0
    for ros in range(rosmin,rosmax):
        for mod in range(modmin, min(modmax,TileCalibUtils.getMaxDrawer(ros))):
            if hexid:
                modName = "0x%x" % ((ros<<8)+mod)
            else:
                modName = TileCalibUtils.getDrawerString(ros,mod)
            if modName in ['EBA39','EBA40','EBA41','EBA42','EBA55','EBA56','EBA57','EBA58',
                           'EBC39','EBC40','EBC41','EBC42','EBC55','EBC56','EBC57','EBC58' ]:
                modSpec = 'EBspC10'
            elif modName in ['EBA15','EBC18']:
                modSpec = 'EBspD4'
            elif modName in ['EBC29','EBC32','EBC34','EBC37']:
                modSpec = 'EBspE4'
            elif modName in ['EBA07', 'EBA25', 'EBA44', 'EBA53',
                             'EBC07', 'EBC25', 'EBC44', 'EBC53',
                             'EBC28', 'EBC31', 'EBC35', 'EBC38' ]:
                modSpec = 'EBspE1'
            elif modName in ['EBA08', 'EBA24', 'EBA43', 'EBA54',
                             'EBC08', 'EBC24', 'EBC43', 'EBC54' ]:
                modSpec = 'EBMBTS'
            else:
                modSpec = modName
            try:
                if oneModule:
                    newBlob = blobReader.getBlob(ros, mod, iovs, False)
                    if oldBlob == newBlob:
                        log.info( f'{modName} in IOV {iovs} is identical to previous IOV')
                        continue
                    else:
                        if oldBlob is None:
                            log.info( f'Reading IOV {iovs} for module {modName}')
                        else:
                            log.info( f'{modName} was updated in IOV {iovs}')
                    oldBlob = newBlob
                flt = blobReader.getDrawer(ros, mod,iovs, False, False)
                if flt is None or isinstance(flt, (int)):
                    miss+=1
                    print ("%s is missing in DB" % modName)
                else:
                    good+=1
                    if blob:
                        print ("%s  Blob type: %d  Version: %d  Nchannels: %d  Ngains: %d  Nval: %d" % (modName, flt.getObjType(), flt.getObjVersion(), flt.getNChans(), flt.getNGains(), flt.getObjSizeUint32()))
                    typeName = TileCalibType.getClassName(flt.getObjType())[-3:]
                    mval0 = 0
                    mval = flt.getObjSizeUint32()
                    if nval<0 and -nval<=mval:
                        mval=-nval
                        mval0=mval-1
                    elif nval!=0 and nval<mval:
                        mval = nval
                    mchan=flt.getNChans()
                    mgain=flt.getNGains()
                    chmin = chanmin if chanmin<mchan else mchan
                    chmax = chanmax if chanmax<mchan else mchan
                    gnmin = gainmin if gainmin<mgain else mgain
                    gnmax = gainmax if gainmax<mgain else mgain
                    for chn in range(chmin,chmax):
                        for adc in range(gnmin,gnmax):
                            if pmt:
                                msg = "%s pm %02i ch %02i %s  " % ( modName, abs(ch2pmt[ros][chn]), chn, gname[adc] )
                            else:
                                msg = "%s %2i %1i  " % ( modName, chn, adc )
                            for val in range(mval0,mval):
                                if str(val) in keep or modName in keep or  modSpec in keep or modName[:3] in keep or  modName[:2] in keep \
                                    or ("%sch%i"% (modName,chn)) in keep or ("%sch%i"% (modSpec,chn)) in keep or ("%sch%i"% (modName[:3],chn)) in keep or ("%sch%i"% (modName[:2],chn)) in keep \
                                    or ("%sch%ig%i"% (modName,chn,adc)) in keep or ("%sch%ig%i"% (modSpec,chn,adc)) in keep or ("%sch%ig%i"% (modName[:3],chn,adc)) in keep or ("%sch%ig%i"% (modName[:2],chn,adc)) in keep:
                                    msg += "   keep   "
                                else:
                                    if typeName=='Int':
                                        v = flt.getData(chn, adc, val)
                                        if v>0xff:
                                            msg += "  0x%x" % v
                                        else:
                                            msg += "  %3d" % v
                                    elif typeName=='Bch':
                                        msg += "  %d" % flt.getData(chn, adc, val)
                                    elif doubl:
                                        msg += "  %s" % flt.getData(chn, adc, val)
                                    else:
                                        msg += "  %f" % flt.getData(chn, adc, val)
                            print (pref+msg)
            except Exception as e:
                print (e)
    if miss:
        if iovs[0]!=2147483647:
            print ("%3i drawers are present in DB for run %d lb %d" % (good,iovs[0],iovs[1]))
            print ("%3i drawers are missing in DB for run %d lb %d" % (miss,iovs[0],iovs[1]))
        else:
            print ("%3i drawers are present in DB" % (good))
            print ("%3i drawers are missing in DB" % (miss))
        if good==0:
            print ("Please, check that you are using correct schema and correct tag")
