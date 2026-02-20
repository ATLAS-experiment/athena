# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys
from re import match
from time import ctime
from typing import Any

from PyCool import cool
from CoolConvUtilities.AtlCoolLib import indirectOpen

def iterate_runlist(runlist: list[tuple[int, int]]):
    """Helper to iterate through runlist. The format is:
    runlist: [[run1,run2], [run3,run4], ... ]
    In addition each "run" can be a tuple of format (run,LB)
    """
    def makeRunLumi(rl):
        """Create (Run,Lumi) tuple"""
        return rl if isinstance(rl,tuple) else (rl,0)

    for (a,b) in runlist:
        (r1,l1) = makeRunLumi(a)
        (r2,l2) = makeRunLumi(b)

        limmin = (r1 << 32) + l1
        if isinstance(b,tuple):
            limmax = (r2 << 32) + l2
        else:
            limmax = ((r2+1) << 32) - 1  # full run

        yield (limmin, limmax)


class TriggerCoolUtil:

    @staticmethod
    def GetConnection(dbconn, verbosity=0):
        connection = None
        m = match(r".*?([^/.]+)\.db",dbconn)
        if dbconn in ["CONDBR2","COMP200","OFLP200"]:
            connection = f'COOLONL_TRIGGER/{dbconn}'
        elif m:
            dbname=m.group(1).upper()
            connection = "sqlite://;schema=%s;dbname=%s;" % (dbconn,dbname)
        else:
            raise RuntimeError ("Can't connect to COOL db %s" % dbconn)
        try:
            openConn = indirectOpen(connection,readOnly=True,debug=(verbosity>0))
        except Exception:
            import traceback
            traceback.print_exc()
            sys.exit(-1)
        return openConn


    @staticmethod
    def getMenuConfigKey(run: int, db = None) -> dict[str, Any] | None:
        if db is None:
            db = TriggerCoolUtil.getDBConnectionForRun(run)
        configs = TriggerCoolUtil.getHLTConfigKeys(db, [(run, run)])
        return configs.get(run, None)

    @staticmethod
    def getHLTConfigKeys(db,runlist):
        configKeys = {}
        f = db.getFolder( "/TRIGGER/HLT/HltConfigKeys" )
        for (limmin,limmax) in iterate_runlist(runlist):
            objs = f.browseObjects( limmin, limmax, cool.ChannelSelection(0))
            while objs.goToNext():
                obj=objs.currentRef()
                runNr   = obj.since()>>32
                if runNr>1e9: continue  # test runs in COOL
                payload=obj.payload()
                smk     = payload['MasterConfigurationKey']
                hltpsk  = payload['HltPrescaleConfigurationKey']
                # Format for ConfigSource (see ATR-21550):
                #   Run-2: TRIGGERDBR2R,21.1.24,AthenaP1
                #   Run-3: TRIGGERDB_RUN3;22.0.101;Athena,22.0.101 --extra_args ...
                release = 'unknown'
                if runNr>379000:
                    confsrc = payload['ConfigSource'].split(';')
                    if len(confsrc)>2: release = confsrc[2].split()[0]
                else:
                    confsrc = payload['ConfigSource'].split(',', maxsplit=1)
                    if len(confsrc)>1: release = confsrc[1]

                dbalias = confsrc[0]
                configKeys[runNr] = { "REL"     : release,
                                      "SMK"     : smk,
                                      "HLTPSK"  : hltpsk,
                                      "DB"      : dbalias }
        return configKeys

    @staticmethod
    def _getKeys(db, runlist, folder, in_name, out_name):
        """Helper to retrieve run/LB-index configuration keys
        returns empty dict if not iov is found
        """
        lbmask = 0xFFFFFFFF
        configKeys = {}
        f = db.getFolder( folder )
        for (limmin,limmax) in iterate_runlist(runlist):
            objs = f.browseObjects( limmin, limmax, cool.ChannelSelection(0))
            while objs.goToNext():
                obj=objs.currentRef()
                runNr   = obj.since()>>32
                if runNr>1e9: continue  # test runs in COOL
                payload = obj.payload()
                key  = payload[in_name]
                firstLB = obj.since() & lbmask
                until = (obj.until() & lbmask)
                lastLB =  until-1 if until>0 else lbmask
                configKeys.setdefault(runNr,{}).setdefault( out_name, [] ).append((key,firstLB,lastLB))

        return configKeys

    @staticmethod
    def getHLTPrescaleKeys(db,runlist):
        return TriggerCoolUtil._getKeys(db, runlist, "/TRIGGER/HLT/PrescaleKey",
                                        "HltPrescaleKey", "HLTPSK2")

    @staticmethod
    def getHLTPrescaleKey(run: int, lb: int, db = None) -> int | None:
        if db is None:
            db = TriggerCoolUtil.getDBConnectionForRun(run)
        prescales = TriggerCoolUtil.getHLTPrescaleKeys(db, [(run, run)])
        if(run not in prescales):
            return None
        for (hltpsk, firstlb, lastlb) in prescales[run]['HLTPSK2']:
            if firstlb<=lb and lb<=lastlb:
                return hltpsk
        return None
    
    @staticmethod
    def getL1ConfigKeys(db,runlist):
        return TriggerCoolUtil._getKeys(db, runlist, "/TRIGGER/LVL1/Lvl1ConfigKey",
                                        "Lvl1PrescaleConfigurationKey", "LVL1PSK")

    @staticmethod
    def getL1PrescaleKey(run: int, lb: int, db = None) -> int | None:
        if db is None:
            db = TriggerCoolUtil.getDBConnectionForRun(run)
        prescales = TriggerCoolUtil.getL1ConfigKeys(db, [(run, run)])
        if(run not in prescales):
            return None
        for (l1psk, firstlb, lastlb) in prescales[run]['LVL1PSK']:
            if firstlb<=lb and lb<=lastlb:
                return l1psk
        return None

    @staticmethod
    def getBunchGroupKey(db,runlist) -> dict[int, Any]:
        return TriggerCoolUtil._getKeys(db, runlist, "/TRIGGER/LVL1/BunchGroupKey",
                                        "Lvl1BunchGroupConfigurationKey", "BGKey")

    @staticmethod
    def getBunchGroupKeyForRunLB(run: int, lb: int, db = None) -> int | None:
        if db is None:
            db = TriggerCoolUtil.getDBConnectionForRun(run)
        bunches = TriggerCoolUtil.getBunchGroupKey(db, [(run, run)])
        if(run not in bunches):
            return None
        for (bgsk, firstlb, lastlb) in bunches[run]['BGKey']:
            if firstlb<=lb and lb<=lastlb:
                return bgsk
        return None

    @staticmethod
    def getDBConnectionForRun(runNumber: int):
        return TriggerCoolUtil.GetConnection('CONDBR2' if runNumber > 230000 else 'COMP200')

    @staticmethod
    def getTrigConfKeys(runNumber: int, lumiBlock: int) -> dict[str, Any]:
        db = TriggerCoolUtil.getDBConnectionForRun(runNumber)
        bgkey =  TriggerCoolUtil.getBunchGroupKeyForRunLB(runNumber, lumiBlock, db)
        l1pskey = TriggerCoolUtil.getL1PrescaleKey(runNumber, lumiBlock, db)
        hltpskey = TriggerCoolUtil.getHLTPrescaleKey(runNumber, lumiBlock, db)
        menucfg = TriggerCoolUtil.getMenuConfigKey(runNumber, db)
        return {
            "SMK": menucfg['SMK'] if menucfg else None,
            "DB": menucfg['DB'] if menucfg else None,
            "LVL1PSK": l1pskey,
            "HLTPSK": hltpskey,
            "BGSK": bgkey
        }

    @staticmethod
    def getRunStartTime(db,runlist, runs):
        latestRunNr=0
        startTime = {}
        f = db.getFolder( "/TRIGGER/LUMI/LBLB" )
        for rr in runlist:
            limmin=(rr[0] << 32)+0
            limmax=((rr[1]+1) << 32)+0
            objs = f.browseObjects( limmin, limmax, cool.ChannelSelection(0) )
            while objs.goToNext():
                obj=objs.currentRef()
                runNr  = obj.since()>>32
                if runNr==latestRunNr: continue
                latestRunNr=runNr
                if runNr not in runs: continue
                payload=obj.payload()
                starttime = payload['StartTime']
                startTime[runNr] = { "STARTTIME" : ctime(starttime/1E9).replace(' ','_') }
        return startTime


    @staticmethod
    def printL1Menu(db, run, verbosity):
        limmin=run<<32
        limmax=(run+1)<<32

        printPrescales = verbosity>0
        printDisabled = verbosity>1
        print ("LVL1 Menu:")
        f = db.getFolder( "/TRIGGER/LVL1/Menu" )
        chansel=cool.ChannelSelection.all()
        objs = f.browseObjects( limmin,limmax,chansel)
        fps = db.getFolder( "/TRIGGER/LVL1/Prescales" )
        objsps = fps.browseObjects( limmin,limmax,chansel)
        itemName = {}
        itemPrescale = {}
        longestName = 0
        while objs.goToNext():
            obj=objs.currentRef()
            channel = obj.channelId()
            payload=obj.payload()
            itemname = payload['ItemName']
            itemName[channel] = itemname
            longestName=max(longestName,len(itemname))
            while objsps.goToNext():
                objps=objsps.currentRef()
                channel = objps.channelId()
                payloadps=objps.payload()
                ps = payloadps['Lvl1Prescale']
                if channel in itemPrescale:
                    itemPrescale[channel] += [ ps ]
                else:
                    itemPrescale[channel] = [ ps ]
        for channel in itemName:
            doPrint = False
            for x in itemPrescale[channel]:
                if x>0 or printDisabled: doPrint = True
            if not doPrint: continue
            if printPrescales:
                print ("%4i: %-*s PS " % (channel, longestName, itemName[channel]), itemPrescale[channel])
            else:
                print ("%4i: %s" % (channel, itemName[channel]))
                

    @staticmethod
    def printHLTMenu(db, run, verbosity, printL2=True, printEF=True):
        limmin=run<<32
        limmax=((run+1)<<32)-1
        print ("HLT Menu:")
        f = db.getFolder( "/TRIGGER/HLT/Menu" )
        chansel=cool.ChannelSelection.all()
        objs = f.browseObjects( limmin,limmax,chansel)
        sizeName=0
        sizePS=0
        sizePT=0
        sizeStr=0
        sizeLow=0
        chainNames = {}
        chainExtraInfo = {}
        while objs.goToNext():
            obj=objs.currentRef()
            payload=obj.payload()
            level    = payload['TriggerLevel']
            if level=='L2' and not printL2: continue
            if level=='EF' and not printEF: continue
            name     = payload['ChainName']
            sizeName=max(sizeName,len(name))
            counter  = payload['ChainCounter']
            chainNames[(level,counter)] = name
            if verbosity>0:
                version  = payload['ChainVersion']
                prescale = payload['Prescale']
                passthr  = payload['PassThrough']
                stream   = payload['StreamInfo']
                lower    = payload['LowerChainName']
                sizePS=max(sizePS,prescale)
                sizePT=max(sizePT,passthr)
                sizeStr=max(sizeStr,len(stream))
                sizeLow=max(sizeLow,len(lower))
                chainExtraInfo[(name,level)] = (version, prescale, passthr, stream, lower)
        sizePS = len("%i"%sizePS)
        sizePT = len("%i"%sizePT)
        for c in sorted(chainNames):
            name = chainNames[c]
            print ("%s %4i: %-*s" % (c[0], c[1], sizeName, name),)
            if verbosity>0:
                (version, prescale, passthr, stream, lower) = chainExtraInfo[(name,c[0])]
                print ("[V %1s, PS %*i, PT %*i, by %-*s , => %-*s ]" %
                       (version, sizePS, prescale, sizePT, passthr, sizeLow, lower, sizeStr, stream), end='')
            print()

    @staticmethod
    def printStreams(db, run, verbosity):
        limmin=run<<32
        limmax=((run+1)<<32)-1
        print ("Used Streams:")
        f = db.getFolder( "/TRIGGER/HLT/Menu" )
        chansel=cool.ChannelSelection.all()
        objs = f.browseObjects( limmin,limmax,chansel)
        streams = set()
        while objs.goToNext():
            obj=objs.currentRef()
            payload=obj.payload()
            streamsOfChain = payload['StreamInfo']
            for streamprescale in streamsOfChain.split(';'):
                streamname = streamprescale.split(',')[0]
                streams.add(streamname)
        for s in sorted(list(streams)):
            print (s)


# Testing
if __name__ == "__main__":
    from pprint import pprint

    db = TriggerCoolUtil.GetConnection('CONDBR2')
    run2 = 363947  # Run-2
    run3 = 435333  # Run-3

    for run in [run2, run3]:
        print("\nFull run:")
        pprint(TriggerCoolUtil.getHLTConfigKeys(db, [[run,run]]))
        pprint(TriggerCoolUtil.getHLTPrescaleKeys(db, [[run,run]]))
        pprint(TriggerCoolUtil.getL1ConfigKeys(db, [[run,run]]))
        pprint(TriggerCoolUtil.getBunchGroupKey(db, [[run,run]]))

    # Detailed tests for Run-3:
    print("\nLB range within run:")
    pprint(TriggerCoolUtil.getHLTPrescaleKeys(db, [[(run3,266),(run3,400)]]))

    print("\nRun range:")
    pprint(TriggerCoolUtil.getHLTPrescaleKeys(db, [[run3,435349]]))

    print("\nMultiple run ranges:")
    pprint(TriggerCoolUtil.getHLTPrescaleKeys(db, [[run3,435349],[435831,435927]]))

    print("\nMultiple run/LB ranges:")
    pprint(TriggerCoolUtil.getHLTPrescaleKeys(db, [[(run3,266),(435349,10)],
                                                   [(435831,466),435927]]))
