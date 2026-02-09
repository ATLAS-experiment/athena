#!/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# File: TileSyncBch.py
# Sanya Solodkov Sanya.Solodkov@cern.ch February 2026
# Purpose: copy bad status from one tag to another (e.g. from UPD4 to ONL) in CREST DB
#
# Usage:
# TileSynchronizeBch.py <TAG1> <TAG2> [MASKONLY] [RUN1] [RUN2] [SCHEMA] [OUTPUT] [AUTHOR]
#   <TAG1> - origin tag (no default), can be UPD1 or UPD4 or exact tag
#   <TAG2> - destination (no default) can be UPD1 or UPD4 or exact tag or ONL (for online tags)
#   [MASKONLY] - if "1" or "yes" or "True", channels are only masked and never unmasked
#   [RUN1] - run number to use for <TAG1> (default = MAXRUN)
#   [RUN2] - run number to use for <TAG2> and for sqlite (default = next run)
#   [SCHEMA] - full schema string for input database (optional)
#   [OUTPUT] - directory for output file "tileCalib.json" of full path with output file name (optional)
#   [AUTHOR] - user name for comment string (optional)

import os,sys

tag1 = "" if len(sys.argv) < 2 else sys.argv[1].rpartition("=")[2]
tag2 = "" if len(sys.argv) < 3 else sys.argv[2].rpartition("=")[2]
opt  = "" if len(sys.argv) < 4 else sys.argv[3].rpartition("=")[2]
run1 = "" if len(sys.argv) < 5 else sys.argv[4].rpartition("=")[2]
run2 = "" if len(sys.argv) < 6 else sys.argv[5].rpartition("=")[2]
schema = "CREST"       if len(sys.argv) < 7 else sys.argv[6]
output = ""            if len(sys.argv) < 8 else sys.argv[7]
author = os.getlogin() if len(sys.argv) < 9 else sys.argv[8]
if not output or output.endswith("/") or os.path.isdir(output):
    output = os.path.join(output,"tileCalib.json")
elif not output.endswith(".json"):
    output += ".json"

from TileCalibBlobPython import TileCalibTools
from TileCalibBlobPython import TileBchCrest
from TileCalibBlobPython.TileCalibCrest import MAXRUN
from TileCalibBlobObjs.Classes import TileCalibUtils, TileBchPrbs, \
     TileBchDecoder

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("SyncBch")
import logging
log.setLevel(logging.DEBUG)


log.info("")
if tag1=="":
    log.error( "Please, use non-empty tag as first parameter (e.g. UPD1)")
    sys.exit(2)
if tag2=="":
    log.error( "Please, use non-empty tag as second parameter (e.g. UPD4)")
    sys.exit(2)
if tag1==tag2:
    log.error( "Please, use different tags as first and second parameter (e.g. UPD1 UPD4)")
    sys.exit(2)

opt=opt[0].upper() if len(opt)>0 else " "
copyall = not (opt=="1" or opt=="Y" or opt=="T" or opt=="B")
if copyall:
    log.info( "Copying all statuses from %s to %s", tag1,tag2)
else:
    log.info( "Copying only BAD statuses from %s to %s", tag1,tag2)

if not run1.isdigit() or int(run1) < 0:
    badrun=run1
    run1=MAXRUN
    if not badrun.isdigit():
        log.info( "First run number was not specified, using maximal possible run number %d for input DB", run1 )
    else:
        log.warning( "First run number %s is bad, using maximal possible run number %d for input DB", badrun, run1)
else:
    run1=int(run1)
if not run2.isdigit() or int(run2) < 0:
    badrun=run2
    run2=TileCalibTools.getNextRunNumber()
    if not badrun.isdigit():
        log.info( "Second run number was not specified, using next run number %d for output DB", run2 )
    else:
        log.warning( "Second run number %s is bad, using next run number %d for output DB", badrun, run2)
    if run2 is None or run2<0:
        log.error( "Still bad run number")
        sys.exit(2)
else:
    run2=int(run2)
if not schema:
    schema="CREST"
log.info( "Using schema  %s", schema)

log.info("")

#===================================================================
#====================== FILL DB BELOW ==============================
#===================================================================

#--- check first tag
folder1 = "/TILE/OFL02/STATUS/ADC"
if "ONL" in tag1.upper():
    log.error( "Copy from ONL tag is not supported")
    sys.exit(2)
if tag1.upper().startswith("TILE"):
    folder1 = ""

#--- create first bad channel manager
mgr1 = TileBchCrest.TileBchMgr()
mgr1.setLogLvl(logging.DEBUG)
log.info("Initializing with offline bad channels at tag=%s and time=%s", tag1, (run1, 0))
mgr1.initialize(schema, folder1, tag1, (run1,0))
tag1 = mgr1.getBlobReader().getTag()

#--- check second tag
online = "ONL" in tag2.upper()
if online:
    folder2 = "/TILE/ONL01/STATUS/ADC"
    if tag2.upper().endswith('-HEAD'):
        tag2 = tag2.upper()
    else:
        tag2=""
else:
    folder2 = "/TILE/OFL02/STATUS/ADC"
if tag2.upper().startswith("TILE"):
    folder2 = ""

#--- create second bad channel manager
mgr2 = TileBchCrest.TileBchMgr()
mgr2.setLogLvl(logging.DEBUG)
mgr2.initialize(schema, folder2, tag2, (run2,0), 2)
tag2 = mgr2.getBlobReader().getTag()

#=== synchronize
comment=""
for ros in range(1,5):
    for mod in range(0,64):
        modName = TileCalibUtils.getDrawerString(ros, mod)
        comm = ""
        for chn in range(0, 48):
            statlo = mgr1.getAdcStatus(ros, mod, chn, 0)
            stathi = mgr1.getAdcStatus(ros, mod, chn, 1)

            statloBefore = mgr2.getAdcProblems(ros,mod,chn,0)
            stathiBefore = mgr2.getAdcProblems(ros,mod,chn,1)

            if online:

                # remove all trigger problems first
                for prb in [TileBchPrbs.TrigGeneralMask,
                            TileBchPrbs.TrigNoGain,
                            TileBchPrbs.TrigHalfGain,
                            TileBchPrbs.TrigNoisy]:
                    mgr2.delAdcProblem(ros, mod, chn, 0, prb)
                    mgr2.delAdcProblem(ros, mod, chn, 1, prb)
                # and now set new trigger problems (if any)
                if not statlo.isGood():
                    prbs = statlo.getPrbs()
                    for prb in prbs:
                        if prb in [TileBchPrbs.TrigGeneralMask,
                                   TileBchPrbs.TrigNoGain,
                                   TileBchPrbs.TrigHalfGain,
                                   TileBchPrbs.TrigNoisy]:
                            mgr2.addAdcProblem(ros, mod, chn, 0, prb)
                            mgr2.addAdcProblem(ros, mod, chn, 1, prb)

                if copyall or statlo.isBad() or stathi.isBad():
                    #--- add IgnoreInHlt if either of the ADCs has isBad
                    #--- add OnlineGeneralMaskAdc if the ADCs has isBad
                    if statlo.isBad() and stathi.isBad():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                    elif statlo.isBad():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                    elif stathi.isBad():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)
                    else:
                        #--- delete IgnoreInHlt and OnlineGeneralMaskAdc if both ADCs are not Bad
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.IgnoredInHlt)
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineGeneralMaskAdc)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.IgnoredInHlt)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineGeneralMaskAdc)

                    #--- add OnlineWrongBCID if either of the ADCs has isWrongBCID
                    if statlo.isWrongBCID() or stathi.isWrongBCID():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)
                    else:
                        #--- delete OnlineWrongBCID if the both ADCs has not isWrongBCID
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineWrongBCID)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineWrongBCID)

                    #--- add OnlineBadTiming if either of the ADCs has isBadTiming
                    if statlo.isBadTiming() or stathi.isBadTiming():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)
                    else:
                        #--- delete OnlineBadTiming if the both ADCs has not isBadTiming
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineBadTiming)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineBadTiming)

                    #--- add OnlineTimingDmuBcOffsetPos if either of the ADCs has isTimingDmuBcOffsetPos
                    if statlo.isTimingDmuBcOffsetPos() or stathi.isTimingDmuBcOffsetPos():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                    else:
                        #--- delete OnlineTimingDmuBcOffsetPos if the both ADCs has not isTimingDmuBcOffsetPos
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetPos)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetPos)

                    #--- add OnlineTimingDmuBcOffsetNeg if either of the ADCs has isTimingDmuBcOffsetNeg
                    if statlo.isTimingDmuBcOffsetNeg() or stathi.isTimingDmuBcOffsetNeg():
                        mgr2.addAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                        mgr2.addAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                    else:
                        #--- delete OnlineTimingDmuBcOffsetNeg if the both ADCs has not isTimingDmuBcOffsetNeg
                        mgr2.delAdcProblem(ros, mod, chn, 0, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
                        mgr2.delAdcProblem(ros, mod, chn, 1, TileBchPrbs.OnlineTimingDmuBcOffsetNeg)
            else:
                if copyall or (statlo.isBad() and not mgr2.getAdcStatus(ros, mod, chn, 0).isBad()):
                    mgr2.setAdcStatus(ros,mod,chn,0,statlo)
                if copyall or (stathi.isBad() and not mgr2.getAdcStatus(ros, mod, chn, 1).isBad()):
                    mgr2.setAdcStatus(ros,mod,chn,1,stathi)

            statloAfter = mgr2.getAdcProblems(ros,mod,chn,0)
            stathiAfter = mgr2.getAdcProblems(ros,mod,chn,1)

            if (statloBefore != statloAfter) or (stathiBefore != stathiAfter):
                pbm = [statloBefore, stathiBefore, statloAfter, stathiAfter]
                #print modName,"%3d 0"%chn,statloBefore,"=>",statloAfter
                #print modName,"%3d 1"%chn,stathiBefore,"=>",stathiAfter
                for adc in range(2):
                    if pbm[adc] != pbm[adc + 2]:
                        msg = ''
                        for pb in range(2):
                            if pb:
                                msg += "  =>"
                            else:
                                msg = "%s %2i %1i " % (modName, chn, adc)
                            prbs = pbm[adc+pb*2]
                            if len(prbs):
                                for prbCode in sorted(prbs.keys()):
                                    prbDesc = prbs[prbCode]
                                    msg += " %5i (%s)" % (prbCode, prbDesc)
                            else:
                                msg += "  GOOD"
                        log.info(msg)
                comm += " ch %d" % chn
        if len(comm):
            comment += " " + modName + comm

#=== commit changes
if len(comment):
    mgr2.commitToDb(output, folder2, tag2, (TileBchDecoder.BitPat_onl01 if online else TileBchDecoder.BitPat_ofl01), author, "synchronizing with %s; updated channels:%s" %(tag1, comment), (run2,0))
else:
    log.warning("Folders are in sync, nothing to update")

