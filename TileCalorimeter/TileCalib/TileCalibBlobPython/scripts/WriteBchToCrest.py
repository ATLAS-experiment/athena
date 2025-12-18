#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File:    WriteBchToCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-10-30
#
# Purpose: Prepare JSON file with new bad channel status
# actual masking/unmasking of channels is done by
# external script specified in --execfile= option
# All modules are always written, for channels which are not
# mentioned in input file previous status is copied from CREST DB
#

import getopt,sys,os,bisect
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ", sys.argv[0]," [OPTION] ... ")
    print ("Update TileCal bad channels in COOL")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify folder to use, default is /TILE/OFL02/STATUS/ADC")
    print ("-t, --tag=      specify tag to use, default is RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, default is 0")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-b, --begin=    specify run number of first iov in multi-iov mode, by default uses very first iov")
    print ("-e, --end=      specify run number of last iov in multi-iov mode, by default uses latest iov")
    print ("-L, --endlumi=  specify lumi block number for last iov in multi-iov mode, default is 0")
    print ("-A, --adjust    in multi-iov mode adjust iov boundaries to nearest iov available in DB, default is False")
    print ("-M, --module=   specify module to use in multi-IOV update, default is all")
    print ("-c, --comment=    specify comment which should be written to DB, in multi-iov mode it is appended to old comment")
    print ("-C, --Comment=    specify comment which should be written to DB, in mutli-iov mode it overwrites old comment")
    print ("-U, --user=       specify username for comment")
    print ("-x, --execfile=   specify python file which should be executed, default is bch.py")
    print ("-i, --inschema=   specify name of input JSON file or CREST_SERVER_PATH")
    print ("-o, --outschema=  specify name of output JSON file, default is tileCalib.json")
    print ("-s, --schema=     the same as --o, --outschema")
    print ("-n, --online      write additional file with online bad channel status")
    print ("-p, --upd4        write additional file with CURRENT UPD4 tag")
    print ("-v, --verbose     verbose mode")

letters = "hr:l:b:e:L:AM:s:i:o:t:f:x:c:C:U:npv"
keywords = ["help","run=","lumi=","begin=","end=","endlumi=","adjust","module=","schema=","inschema=","outschema=","tag=","folder=","execfile=","comment=","Comment=","user=","online","upd4","verbose"]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:], letters, keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run = -1
lumi = 0
inSchema = 'CREST'
outSchema = 'tileCalib.json'
folderPath =  "/TILE/OFL02/STATUS/ADC"
onlSuffix = None
curSuffix = None
tag = "UPD1"
execFile = "bch.py"
comment = ""
Comment = None
verbose = False
iov = False
beg = 0
end = 2147483647
endlumi = 0
moduleList = ['ALL']
adjust = False

try:
    user=os.getlogin()
except Exception:
    user="UnknownUser"

for o, a in opts:
    a = a.strip()
    if o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-s","--schema"):
        outSchema = a
    elif o in ("-i","--inschema"):
        inSchema = a
    elif o in ("-o","--outschema"):
        outSchema = a
    elif o in ("-n","--online"):
        onlSuffix = True
    elif o in ("-p","--upd4"):
        curSuffix = True
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-b","--begin"):
        beg = int(a)
        iov = True
    elif o in ("-e","--end"):
        end = int(a)
        iov = True
    elif o in ("-L","--endlumi"):
        endlumi = int(a)
    elif o in ("-A","--adjust"):
        adjust = True
    elif o in ("-M","--module"):
        moduleList = a.split(",")
    elif o in ("-x","--execfile"):
        execFile = a
    elif o in ("-c","--comment"):
        comment = a
    elif o in ("-C","--Comment"):
        Comment = a
        comment = a
    elif o in ("-U","--user"):
        user = a
    elif o in ("-v","--verbose"):
        verbose = True
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        raise RuntimeError("unhandeled option")

onl=("/TILE/ONL01" in folderPath)

from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython import TileCalibTools
from TileCalibBlobPython import TileBchCrest
from TileCalibBlobObjs.Classes import TileBchPrbs, TileBchDecoder

if iov and end >= TileCalibCrest.MAXRUN:
    end = TileCalibCrest.MAXRUN
    endlumi = TileCalibCrest.MAXLBK
until = (TileCalibCrest.MAXRUN,TileCalibCrest.MAXLBK)

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("WriteBchToCrest")
import logging
logLevel=logging.DEBUG
log.setLevel(logLevel)

if tag.upper().endswith('HEAD'):
    tag=tag.upper()
if len(tag)==0 or tag.endswith('HEAD'):
    folderPath='/TILE/ONL01/STATUS/ADC'
    log.info("tag is %s, using %s folder", tag if tag else 'empty', folderPath)
    tag='TILEONL01STATUSADC-HEAD'

folderTag = tag
if folderTag.upper().startswith("TILE") or folderTag.upper().startswith("CALO"):
    folderPath=""
log.info("Initializing bad channels from %s folder %s with tag %s", inSchema, folderPath, folderTag)

iovList = []
iovUntil = []
blobReader = TileCalibCrest.TileBlobReaderCrest(inSchema, folderPath, folderTag, max(run,beg), lumi, 0, 0)
if folderPath and not (folderTag.upper().startswith("TILE") or folderTag.upper().startswith("CALO")):
    folderTag = blobReader.getFolderTag(folderPath,None,folderTag)
if iov:
    #=== filling the iovList
    log.info( "Looking for IOVs" )
    iovList = blobReader.getIovs()
    iovList+=[until]

    since=(beg,lumi)
    ib=bisect.bisect(iovList,since)-1
    if ib<0:
        ib=0
    if iovList[ib] != since:
        if adjust:
            since = iovList[ib]
            log.info( "Moving beginning of first IOV with new bad channels from (%d,%d) to (%d,%d)", beg,lumi,since[0],since[1])
        else:
            iovList[ib] = since
            log.info( "Creating new IOV starting from (%d,%d) with new bad channels", beg,lumi)

    if end<0:
        ie=ib+1
        if ie>=len(iovList):
            ie=ib
        until=iovList[ie]
        log.info( "Next IOV without new bad channels starts from (%d,%d)", until[0],until[1])
    else:
        until=(end,endlumi)
        ie=bisect.bisect_left(iovList,until)
        if ie>=len(iovList):
            ie=len(iovList)-1

        if iovList[ie] != until:
            if adjust:
                until=iovList[ie]
                log.info( "Moving end of last IOV from (%d,%d) to (%d,%d)", end,endlumi,until[0],until[1])
            else:
                log.info( "Keeping end of last IOV at (%d,%d) - new IOV is shorter than IOV in input DB", end,endlumi)
                iovList[ie] = until


    iovList = iovList[ib:ie]
    iovUntil = iovList[1:] + [until]
    begin = since
    run = since[0]
    lumi = since[1]
    log.info( "IOVs: %s", str(iovList) )

    log.info( "%d IOVs in total, end of last IOV is %s", ie-ib,str(until))

else:
    #=== set run number
    if run<0:
        lumi=0
        if "UPD4" in folderTag:
            run=TileCalibTools.getPromptCalibRunNumber()
            log.warning( "Run number is not specified, using minimal run number in calibration loop %d", run )
        else:
            run=TileCalibTools.getLastRunNumber()
            log.warning( "Run number is not specified, using current run number %d", run )
        if run<0:
            log.error( "Bad run number" )
            sys.exit(2)

    since = (run, lumi)
    iovList = [since]
    iovUntil = [until]
    begin=since

    log.info("Initializing for run %d, lumiblock %d", run,lumi)

#=== create bad channel manager
log.info("")
comments = []
mgrWriters = []
nvalUpdated = []
commentsSplit = []
for since in iovList:
    comm=blobReader.getComment(since)
    #log.info("Comment: %s", comm)
    comments+=[comm]
    nvalUpdated += [0]
    commentsSplit+=[blobReader.getComment(since,True)]
    mgr = TileBchCrest.TileBchMgr()
    mgr.setLogLvl(logLevel)
    mgr.initialize(inSchema, folderPath, folderTag, since)
    mgrWriters += [mgr]
log.info("")

#=== Tuples of empty channels
emptyChannelLongBarrel =     (30, 31, 43)
emptyChannelExtendedBarrel = (18, 19, 24, 25, 26, 27, 28, 29, 33, 34, 42, 43, 44, 45, 46, 47)

# remember: addAdcProblem(ros, module, channel, adc), where:
# ros = 1 LBA
# ros = 2 LBC
# ros = 3 EBA
# ros = 4 EBC
# module = 0 - 63
# channel = 0 - 47
# adc: 0 = low gain, 1 = high gain.

#=== print bad channels
if verbose and not iov:
    log.info("============================================================== ")
    log.info("bad channels before update")
    mgr.listBadAdcs()

#=== Add problems with mgr.addAdcProblem(ros, drawer, channel, adc, problem)
#=== Remove problems with mgr.delAdcProblem(ros, drawer, channel, adc, problem)


if len(execFile):
    log.info("Masking new bad channels, including file %s", execFile )

    #=== loop over all IOVs
    for io,since in enumerate(iovList):

        until=iovUntil[io]
        if since==until:
            continue # nothing to do

        log.info( "Updating IOV %s - %s", str(since), str(until) )
        mgr = mgrWriters[io]

        try:
            exec(compile(open(execFile).read(),execFile,'exec'))
            if len(comment)==0:
                log.error( "Comment string is not provided, please put comment='bla-bla-bla' line in %s", execFile )
                sys.exit(2)
        except Exception as e:
            log.error( e )
            log.error( "Problem reading include file %s", execFile )
            sys.exit(2)

        #=== print bad channels
        if verbose:
            log.info("============================================================== ")
            log.info("bad channels after update")
            mgr.listBadAdcs()

        #====================== Write new bad channel list =================

        #=== commit changes
        if Comment is not None:
            comment = Comment
            author = user
        else:
            if comment=="None":
                comment = comments[io]
            elif iov and comments[io] not in comment:
                comment += "  //  " + comments[io]
            if io>0 and since!=until and 'ALL' not in moduleList:
                author=commentsSplit[io]
                for m in moduleList:
                    if m in comments[io]:
                        author=user
                        break
            else:
                author=user
        mgr.commitToDb(outSchema, folderPath, folderTag, (TileBchDecoder.BitPat_onl01 if onl else TileBchDecoder.BitPat_ofl01), author, comment, since, moduleList)


since = iovList[0]
until = (TileCalibCrest.MAXRUN,TileCalibCrest.MAXLBK)

if curSuffix and not onl:

    if len(comment) == 0:
        comment=blobReader.getComment(since)
        if comment.find("): ") > -1:
            comment = comment[(comment.find("): ")) + 3:]

    log.info("")
    log.info("============================================================== ")
    log.info("")
    log.info("creating DB with CURRENT UPD4 tag")

    folderTagUPD4 = blobReader.getFolderTag(folderPath, None, "UPD4" )
    if folderTagUPD4 == folderTag:
        log.warning("CURRENT UPD4 tag %s is identical to the tag in DB which was created already", folderTagUPD4)
        folderTagUPD4 = blobReader.getFolderTag(folderPath, None, "UPD1" )
        log.warning("Additional UPD1 DB with tag %s will be created instead", folderTagUPD4 )

    mgr.updateFromDb(inSchema, folderPath, folderTagUPD4, since, 0)

    #=== commit changes
    mgr.commitToDb(outSchema, folderPath, folderTagUPD4, TileBchDecoder.BitPat_ofl01, user, comment, since, moduleList)


if onlSuffix and not onl:

    if len(comment)==0:
        comment = blobReader.getComment(since)
        if comment.find("): ") > -1:
            comment = comment[(comment.find("): ")) + 3:]

    log.info("")
    log.info("============================================================== ")
    log.info("")
    log.info("creating DB with ONLINE status")

    #--- create online bad channel manager
    folderOnl = "/TILE/ONL01/STATUS/ADC"
    folderTagOnl = "TILEONL01STATUSADC-HEAD"

    mgrOnl = TileBchCrest.TileBchMgr()
    mgrOnl.setLogLvl(logLevel)
    mgrOnl.initialize(inSchema, folderOnl, folderTagOnl, since)

    #=== print online channel status
    if verbose:
        log.info("============================================================== ")
        log.info("online channel status BEFORE update")
        mgrOnl.listBadAdcs()

    #=== synchronize
    for ros in range(1, 5):
        for mod in range(0, 64):
            for chn in range(0, 48):
                statlo = mgr.getAdcStatus(ros, mod, chn, 0)
                stathi = mgr.getAdcStatus(ros, mod, chn, 1)

                # remove all trigger problems first
                for prb in [TileBchPrbs.TrigGeneralMask,
                            TileBchPrbs.TrigNoGain,
                            TileBchPrbs.TrigHalfGain,
                            TileBchPrbs.TrigNoisy]:
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, prb)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, prb)
                # and now set new trigger problems (if any)
                if not statlo.isGood():
                    prbs = statlo.getPrbs()
                    for prb in prbs:
                        if prb in [TileBchPrbs.TrigGeneralMask,
                                   TileBchPrbs.TrigNoGain,
                                   TileBchPrbs.TrigHalfGain,
                                   TileBchPrbs.TrigNoisy]:
                            mgrOnl.addAdcProblem(ros, mod, chn, 0, prb)
                            mgrOnl.addAdcProblem(ros, mod, chn, 1, prb)

                #--- add IgnoreInHlt if either of the ADCs has isBad
                #--- add OnlineGeneralMaskAdc if the ADCs has isBad
                if statlo.isBad() and stathi.isBad():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                elif statlo.isBad():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                elif stathi.isBad():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                else:
                    #--- delete IgnoreInHlt and OnlineGeneralMaskAdc if both ADCs are not Bad
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)

                #--- add OnlineBadTiming if either of the ADCs has isBadTiming
                if statlo.isBadTiming() or stathi.isBadTiming():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)
                else:
                    #--- delete OnlineBadTiming if the both ADCs has not isBadTiming
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)

                #--- add OnlineTimingDmuBcOffsetPos if either of the ADCs has isTimingDmuBcOffsetPos
                if statlo.isTimingDmuBcOffsetPos() or stathi.isTimingDmuBcOffsetPos():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                else:
                    #--- delete OnlineTimingDmuBcOffsetPos if the both ADCs has not isTimingDmuBcOffsetPos
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)

                #--- add OnlineTimingDmuBcOffsetNeg if either of the ADCs has isTimingDmuBcOffsetNeg
                if statlo.isTimingDmuBcOffsetNeg() or stathi.isTimingDmuBcOffsetNeg():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                else:
                    #--- delete OnlineTimingDmuBcOffsetNeg if the both ADCs has not isTimingDmuBcOffsetNeg
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)

                #--- add OnlineWrongBCID if either of the ADCs has isWrongBCID
                if statlo.isWrongBCID() or stathi.isWrongBCID():
                    mgrOnl.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                    mgrOnl.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)
                else:
                    #--- delete OnlineWrongBCID if the both ADCs has not isWrongBCID
                    mgrOnl.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                    mgrOnl.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)


    #=== print online channel status
    if verbose:
        log.info("online channel status AFTER update")
        mgrOnl.listBadAdcs()

    #=== commit changes
    mgrOnl.commitToDb(outSchema, folderOnl, folderTagOnl, TileBchDecoder.BitPat_onl01, user, comment, since, moduleList)
