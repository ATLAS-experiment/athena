# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__author__  = 'Javier Montejo'
__version__="$Revision: 2.0 $"
__doc__="Access to Trigger DB and TriggerMenu to read past and future prescales"

import itertools
import sys
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, LBexceptions, TriggerRenaming
from TriggerMenuMT.TriggerAPI.TriggerPeriodData import TriggerPeriodData
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

import os

def getRunLBFromU64(runlb):
    run = runlb >> 32
    lb = runlb & ((1<<32)-1)
    return ( int(run), int(lb) )


def getReadyForPhysicsInRange(period):
    """
    returns all runs in the given period which have the ReadyForPhysics flag set in at least 1 LB
    """

    log.info("Loading COOL libs...")
    from CoolLumiUtilities.CoolDataReader import CoolDataReader
    log.info("Done loading libs, starting now ...")

    myReader = CoolDataReader('COOLONL_TDAQ/CONDBR2', '/TDAQ/RunCtrl/DataTakingMode')
    runsWithReady = {}

    firstRun = min([x for x in period.keys()])
    lastRun = max([x for x in period.keys()])+1
    since = (firstRun << 32)
    until = (lastRun << 32)

    myReader.setIOVRange( since, until )
    myReader.readData()
    
    for obj in myReader.data:
        isReady = (obj.payload()['ReadyForPhysics'] == 1)
        if not isReady:
            continue
        sincerun, sincelb = getRunLBFromU64(obj.since())
        untilrun, untillb = getRunLBFromU64(obj.until())
        if sincerun != untilrun:
            log.info("WARNING: ready block crosses run boundaries: %d %d", sincerun, untilrun)
        if sincerun not in period: continue
        if sincerun in runsWithReady:
            runsWithReady[sincerun] += [ (sincelb, untillb) ]
        else:
            runsWithReady[sincerun] = [ (sincelb, untillb) ]


    return runsWithReady


def getKeys( listOfRuns, doPrint = False ):

    from CoolLumiUtilities.CoolDataReader import CoolDataReader

    keysByRun = {}

    mySmkReader = CoolDataReader('COOLONL_TRIGGER/CONDBR2', '/TRIGGER/HLT/HltConfigKeys')
    myL1pskReader = CoolDataReader('COOLONL_TRIGGER/CONDBR2', '/TRIGGER/LVL1/Lvl1ConfigKey')
    myHltpskReader = CoolDataReader('COOLONL_TRIGGER/CONDBR2', '/TRIGGER/HLT/PrescaleKey')
    #myBgskReader = CoolDataReader('COOLONL_TRIGGER/CONDBR2', '/TRIGGER/LVL1/BunchGroupKey')

    import tqdm
    pbar = tqdm.tqdm(sorted(listOfRuns),unit=" runs",bar_format='{l_bar}{bar:10}{r_bar}{bar:-10b}')
    for run in pbar:

        listOfReadyBlocks = listOfRuns[run]

        pbar.set_description(f"Getting keys for run {run}")

        since = (run << 32) 
        until = ((run+1) << 32)

        # super master key
        mySmkReader.setIOVRange( since, until - 1 )
        mySmkReader.readData()
        for obj in mySmkReader.data:
            smk = obj.payload()['MasterConfigurationKey']
            sincerun, sincelb = getRunLBFromU64(obj.since())
            untilrun, untillb = getRunLBFromU64(obj.until())
            keysByRun.setdefault(run,{})['smk'] = smk

        for sincelb, untillb in listOfReadyBlocks:

            since = (run << 32) + sincelb
            until = (run << 32) + untillb

            # l1 prescale keys
            myL1pskReader.setIOVRange( since, until )
            myL1pskReader.readData()
            for obj in myL1pskReader.data:
                l1psk = obj.payload()['Lvl1PrescaleConfigurationKey']
                sincerun2, sincelb2 = getRunLBFromU64(obj.since())
                untilrun2, untillb2 = getRunLBFromU64(obj.until())
                if sincelb2 == untillb: break
                keysByRun.setdefault(run,{}).setdefault('l1psk',[]).append((l1psk,sincerun2, sincelb2,untilrun2, untillb2-1)) #use same convention as GRL, last lb is included
            
            # hlt prescale keys
            myHltpskReader.setIOVRange( since, until )
            myHltpskReader.readData()
            for obj in myHltpskReader.data:
                hltpsk = obj.payload()['HltPrescaleKey']
                sincerun2, sincelb2 = getRunLBFromU64(obj.since())
                untilrun2, untillb2 = getRunLBFromU64(obj.until())
                if sincelb2 == untillb: break
                keysByRun.setdefault(run,{}).setdefault('hltpsk',[]).append((hltpsk,sincerun2, sincelb2,untilrun2, untillb2-1)) #use same convention as GRL, last lb is included

            ## Bunch group key
            #myBgskReader.setIOVRange( since, until )
            #myBgskReader.readData()
            #for obj in myBgskReader.data:
            #    bgsk = obj.payload()['Lvl1BunchGroupConfigurationKey']
            #    sincerun2, sincelb2 = getRunLBFromU64(obj.since())
            #    untilrun2, untillb2 = getRunLBFromU64(obj.until())
            #    keysByRun.setdefault(run,{}).setdefault('bgsk',[]).append(bgsk)

    if doPrint:
        log.info(keysByRun)

    return keysByRun

def getHLTPrescalesRun2(connection,psk):
    """returns set name, prescale and passthrough 
    values for a given HLT prescale key 
    @connection - connection string, e.g. TRIGGERDB
    @psk - HLT prescale key
    @return (ps name, [('L2/EF',chainId,prescale,pass-through),...])
    """

    res = queryHLTPrescaleTableRun2(connection,psk)
    prescales = {r[0]:r[3] for r in res if r and r[1]=='Prescale'}
    rerun     = {r[0]:r[3] for r in res if r and r[1]=='ReRun'}
    for x in prescales:
        if x not in rerun: rerun[x] = False
    return {x: (prescales[x],rerun[x]) for x in prescales}

def queryHLTPrescaleTableRun2(connection,psk):

    from TrigConfigSvc.TrigConfigSvcUtils import getTriggerDBCursor, executeQuery

    global cursor, schemaname
    cursor,schemaname = getTriggerDBCursor(connection)

    output = [ "PS.HPR_CHAIN_COUNTER", "PS.HPR_TYPE", "PS.HPR_CONDITION" , "PS.HPR_VALUE"]

    tables = {}
    tables['PS'] = 'HLT_PRESCALE' 

    condition = [ "PS.HPR_PRESCALE_SET_ID = :psk" ]

    bindvars = { "psk": psk }

    res = executeQuery(cursor, output, condition, schemaname, tables, bindvars)

    return res


def fillHLTmap( info, hltMap_prev , lbCount, run, grlblocks):
    from TrigConfigSvc.TrigConfigSvcUtils import getL1Items, getL1Prescales

    from TrigConfIO.L1TriggerConfigAccess import L1MenuAccess,L1PrescalesSetAccess # run3 menu access
    from TrigConfIO.HLTTriggerConfigAccess import HLTMenuAccess,HLTPrescalesSetAccess # run3 menu access

    from collections import defaultdict

    # for run2 or earlier, need to quieten output of TrigConfigSvcUtils
    tcsLogger = logging.getLogger("TrigConfigSvcUtils.py")
    tcsLogLevel = tcsLogger.level
    tcsLogger.setLevel(logging.ERROR)

    lvl = int(logging.root.level)
    logging.root.setLevel(logging.WARNING)

    # obtain map l1 item name => CTP ID
    # obtain map hltid => (hltname, l1seed)
    if run > 400000:
        items = {}; chainsHLT = {}
        for name,value in L1MenuAccess(dbalias = 'TRIGGERDB_RUN3', smkey = info['smk']).items().items():
            items[name] = value["ctpid"]
        for name,value in HLTMenuAccess(dbalias = 'TRIGGERDB_RUN3', smkey = info['smk']).chains().items():
            if "L1" not in value["l1item"]: continue # filtering
            chainsHLT[value["nameHash"]] = (name,value["l1item"])
    else:
        items = getL1Items('oracle://ATLAS_CONFIG/ATLAS_CONF_TRIGGER_RUN2', info['smk'])
        chainsHLT = getChainsWithL1seed('oracle://ATLAS_CONFIG/ATLAS_CONF_TRIGGER_RUN2', info['smk']) # returns map HLT ID => (HLT name, L1 seed)
        chainsHLT = {k:v for (k,v) in chainsHLT.items() if "L1" in v[1]} # filtering



    tmphltList = []
    for lbrange in info['hltpsk']:
        lbstart, lbend = lbrange[2], lbrange[4]
        if lbend ==-1: lbend = 2000
        if run > 400000:
            hltprescales = {}
            for name,value in HLTPrescalesSetAccess(dbalias='TRIGGERDB_RUN3',hltpskey=lbrange[0]).prescales().items():
                rerun=-1.0 # how to determine?
                # key seems to be a float (from looking at type(list(getHLTPrescalesRun2("TRIGGERDB",3000).keys())[0]))
                # ATR-30554: There is at least one instance in smk:3393 hltpsk:11710 where the prescale is string encoded
                hltprescales[value["hash"]] = (float(value["prescale"]),rerun)
        else:
            hltprescales = getHLTPrescalesRun2('oracle://ATLAS_CONFIG/ATLAS_CONF_TRIGGER_RUN2', lbrange[0])
        tmphltList.append(( lbstart, lbend,hltprescales) )

    tmpl1List = []
    for lbrange in info['l1psk']:
        lbstart, lbend = lbrange[2], lbrange[4]
        if lbend ==-1: lbend = 2000
        if run > 400000:
            l1ps = L1PrescalesSetAccess(dbalias='TRIGGERDB_RUN3',l1pskey=lbrange[0])
            l1prescales    = {name: l1ps.prescale(name) for name in l1ps.itemNames()}
        else:
            l1psname, l1prescales = getL1Prescales('oracle://ATLAS_CONFIG/ATLAS_CONF_TRIGGER_RUN2', lbrange[0])
            l1prescales = list(l1prescales)
            l1prescales    = {l1name: l1prescales[int(l1id)] for (l1name, l1id) in items.items()}
        tmpl1List.append(( lbstart, lbend,l1prescales) )

    logging.root.setLevel(lvl)
    tcsLogger.setLevel(tcsLogLevel)

    #merge the lb ranges of HLT and L1
    hltindex, l1index = 0,0
    mergedList = []
    while hltindex < len(tmphltList) and l1index < len(tmpl1List) :
        if tmphltList[hltindex][1] == tmpl1List[l1index][1]:
            lbstart, lbend = max(tmphltList[hltindex][0],tmpl1List[l1index][0]), tmphltList[hltindex][1]
            mergedList.append((lbstart, lbend,tmphltList[hltindex][2],tmpl1List[l1index][2]))
            hltindex += 1
            l1index += 1
        elif tmphltList[hltindex][1] > tmpl1List[l1index][1]:
            lbstart, lbend = max(tmphltList[hltindex][0],tmpl1List[l1index][0]), tmpl1List[l1index][1]
            mergedList.append((lbstart, lbend,tmphltList[hltindex][2],tmpl1List[l1index][2]))
            l1index += 1
        else:
            lbstart, lbend = max(tmphltList[hltindex][0],tmpl1List[l1index][0]), tmphltList[hltindex][1]
            mergedList.append((lbstart, lbend,tmphltList[hltindex][2],tmpl1List[l1index][2]))
            hltindex += 1

    # if user does a "touch liveFractions.txt" then we will populate info to that file for further debugging
    f = open("liveFractions.txt","a") if os.path.exists("liveFractions.txt") else None

    hltMap = {}
    for lbstart, lbend, hltprescales, l1prescales in mergedList:
        if run in LBexceptions.exceptions:
            if any([lbstart>=exc_start and lbstart<=exc_end for exc_start, exc_end in LBexceptions.exceptions[run]]): continue
            if any([lbend>=exc_start and lbend<=exc_end for exc_start, exc_end in LBexceptions.exceptions[run]]): continue

        for grllbstart,grllbend in grlblocks:
            lboverlap = (min(lbend,grllbend) - max(lbstart,grllbstart))+1
            if lboverlap <= 0: continue

            lbCount += lboverlap
            for hltid, (hltps, hltrerun) in hltprescales.items():
                if hltid not in chainsHLT: continue
                if hltps < 1: hltps = 1e99
                l1seeds = chainsHLT[hltid][1]
                l1ps = 1e99
                for l1seed in l1seeds.split(","): #protect 'L1_MU20,L1_MU21'
                    if l1seed not in l1prescales and len(l1seeds) > 10: continue #Protection against buggy HLT_noalg_Standby
                    tmpl1ps = l1prescales[l1seed]
                    if tmpl1ps < 1: tmpl1ps = 1e99
                    l1ps = min(l1ps, tmpl1ps)


                if hltps*l1ps < 1e99: efflb = lboverlap/(hltps*l1ps)
                else:                 efflb = 0
                if not chainsHLT[hltid][0] in hltMap: hltMap[chainsHLT[hltid][0]] = [l1seeds, 0, hltrerun>0, defaultdict(int)]
                hltMap[chainsHLT[hltid][0]][1] += efflb
                hltMap[chainsHLT[hltid][0]][3][run] += efflb
                if f: f.write(f"{chainsHLT[hltid][0]},{run},{lbstart},{lbend},{grllbstart},{grllbend},{lboverlap},{l1ps},{hltps}\n")

    if f: f.close()
    
    for hlt,(l1,efflb,rerun,efflbByRun) in hltMap_prev.items():
        if hlt in hltMap: 
            hltMap[hlt][1] += efflb
            hltMap[hlt][2] |= rerun
            for run,runefflb in efflbByRun.items():
                hltMap[hlt][3][run] += runefflb
        else: hltMap[hlt] = [l1, efflb,rerun, efflbByRun]
    return hltMap, lbCount


def getChainsWithL1seed(connection, smk):
    '''copy of getChains but retrieving also the L1 seed and assuming always run2
         https://gitlab.cern.ch/atlas/athena/blob/master/Trigger/TrigConfiguration/TrigConfigSvc/python/TrigConfigSvcUtils.py#L611
    '''
    from TrigConfigSvc.TrigConfigSvcUtils import getTriggerDBCursor, executeQuery
    cursor,schemaname = getTriggerDBCursor(connection)
         
    output = ['TC.HTC_ID', 'TC.HTC_CHAIN_COUNTER', 'TC.HTC_NAME', 'TC.HTC_LOWER_CHAIN_NAME']
    tables = {}
    tables['SM']    = 'SUPER_MASTER_TABLE'
    tables['M2C']     = 'HLT_TM_TO_TC'
    tables['TC']    = 'HLT_TRIGGER_CHAIN'
    tables['MT']    = 'HLT_MASTER_TABLE'

    condition = [ "SM.SMT_ID = :smk",
                    'SM.SMT_HLT_MASTER_TABLE_ID = MT.HMT_ID',
                    'MT.HMT_TRIGGER_MENU_ID = M2C.HTM2TC_TRIGGER_MENU_ID',
                    'M2C.HTM2TC_TRIGGER_CHAIN_ID = TC.HTC_ID' ]
    
    bindvars = { "smk": smk }
    
    res = executeQuery(cursor, output, condition, schemaname, tables, bindvars)
    
    chainsef = {}
    for x in res:
        if len(x)!=4: continue #protect against HLT_noalg_bkg_L1Bkg and similars
        chainsef[x[1]] = (x[2],x[3])

    return chainsef



def getHLTmap_fromDB(period, customGRL):
    ''' Return a map of HLT chain: (L1 seed, active LBs, is-rerun) for a given period
    '''
    
    triggerPeriod = TriggerPeriodData( period, customGRL ).grl
    if not triggerPeriod: return {},0
    runsWithReadyForPhysics = getReadyForPhysicsInRange(triggerPeriod)
    keys = getKeys( runsWithReadyForPhysics)
    
    hltMap = {}
    lbCount = 0
    lbByRun = {}
    import tqdm
    pbar = tqdm.tqdm(keys,unit=" runs",bar_format='{l_bar}{bar:10}{r_bar}{bar:-10b}')
    for run in pbar:
        pbar.set_description(f"Getting prescales for run {run}")
        prev_lbCount = int(lbCount)
        hltMap, lbCount = fillHLTmap( keys[run], hltMap, lbCount , run, triggerPeriod[run])
        lbByRun[run] = (lbCount - prev_lbCount)

    return hltMap, lbCount, lbByRun

def getHLTmap_fromTM(flags, period, release):
    ''' Return a map of HLT chain: (L1 seed, active LBs, is-rerun) for a given period
        Only "Future" periods make sense here
        The format is the same as for TriggerDBAccess for compatibility but rerun is always false
    '''

    if not period & TriggerPeriod.future: return {}, 0
    hltMap = {}
    dummyfutureLBs = 1e6

    from TrigConfigSvc.TrigConfigSvcCfg import getTrigConfigFromFlag
    if (flags.Input.Files and os.path.exists(flags.Input.Files[0]) and getTrigConfigFromFlag(flags)["SOURCE"]=='INFILE' and
        (flags.Input.isMC or flags.Input.DataYear>=2022) ): # R2 data should not use this infile menu for now, to keep outputs the same as before this feature was added
        # get menu out of the first input file
        from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
        try:
            menu = getHLTMenuAccess(flags)
            for hltname,chain in menu.chains().items():
                l1seed  = chain["l1item"]
                if l1seed == "": l1seed = "All"
                primary = any('Primary' in g or 'TagAndProbe' in g for g in chain["groups"])
                ps = 1 if primary else 0
                hltMap[hltname] = (l1seed, dummyfutureLBs*ps, False, {})  #third arg is hasRerun=False
        except RuntimeError:
            log.info("Failed to read infile menu, which can happen with old MC, reverting to release menu")

    if not hltMap:
        # use the generated menus from the release, and pick the menu given according to the flags (Trigger.triggerMenuSetup)
        from TriggerMenuMT.HLT.Config.GenerateMenuMT import GenerateMenuMT
        menu = GenerateMenuMT()
        menu.getChainsFromMenu(flags)
        for chain in itertools.chain.from_iterable(menu.chainsInMenu.values()):
            hltname = chain.name
            l1seed  = chain.name[chain.name.rfind("_L1")+3:] #surely a better way to do this
            primary = any('Primary' in g or 'TagAndProbe' in g for g in chain.groups)
            ps = 1 if primary else 0
            hltMap[hltname] = (l1seed, dummyfutureLBs*ps, False, {})  #third arg is hasRerun=False

    return hltMap, dummyfutureLBs, {}

def getMenuPathFromRelease(release):
    if release: #already format-proofed in TriggerAPI
        return "/cvmfs/atlas.cern.ch/repo/sw/software/21.1/AthenaP1/%s/InstallArea/x86_64-slc6-gcc62-opt/python/TriggerMenu/menu"%release
    #21.1.50 contains the final menu, no need to find the last release
    return "/cvmfs/atlas.cern.ch/repo/sw/software/21.1/AthenaP1/21.1.50/InstallArea/x86_64-slc6-gcc62-opt/python/TriggerMenu/menu"


def getHLTlist(period, customGRL, release, flags=None):
    ''' For a given period it returns: [HLT chain, L1 seed, average livefraction, active LB, is-rerun], total LB
        The average livefraction is an approximation weighting the PS by number of lumiblocks.
        *** Don't use this number in analysis!!! ***
        For "future" periods, the average livefraction is 1 for items flagged as primary in TM and 0 for non-primaries
    '''
    if not period & TriggerPeriod.future or TriggerPeriod.isRunNumber(period): 
        hltmap, totalLB, totalLBByRun = getHLTmap_fromDB(period, customGRL)
        # # add empty chainGroups list to every item
        # for name,vals in hltmap.items():
        #     hltmap[name] = vals + [[],]
    else:
        if flags is None:
            raise RuntimeError('ConfigFlags need to be provided via TriggerAPI.setConfigFlags for "future" periods.')
        hltmap, totalLB, totalLBByRun = getHLTmap_fromTM(flags, period, release)
    
    hltlist = cleanHLTmap(hltmap, totalLB)
    return (hltlist, totalLB, totalLBByRun)

def cleanHLTmap(hltmap, totalLB):

    from copy import deepcopy
    for  name, (l1seed, activeLB, hasRerun,activLBByRun) in deepcopy(hltmap).items(): #since it will modify on the fly
        for pair in TriggerRenaming.pairs:
            if name==pair[0] and     pair[1] in hltmap:
                hltmap[pair[1]][1] += activeLB
                for run,efflb in activLBByRun.items(): hltmap[pair[1]][3][run] += efflb
            #if name==pair[0] and not pair[1] in hltmap: hltmap[pair[1]]     = [l1seed, activeLB, hasRerun]
            if name==pair[1] and     pair[0] in hltmap:
                hltmap[pair[0]][1] += activeLB
                for run,efflb in activLBByRun.items(): hltmap[pair[0]][3][run] += efflb
            #if name==pair[1] and not pair[0] in hltmap: hltmap[pair[0]]     = [l1seed, activeLB, hasRerun]

    vetoes = ['calib','noise','noalg','satmon','peb']
    hltlist = [(name, l1seed, activeLB/totalLB, activeLB, hasRerun,activLBByRun) for name, (l1seed, activeLB, hasRerun,activLBByRun) in hltmap.items() if not any(v in name for v in vetoes)]
    return hltlist

def test():
    log.info(getHLTlist(TriggerPeriod.future,None, None))

if __name__ == "__main__":
    sys.exit(test())
