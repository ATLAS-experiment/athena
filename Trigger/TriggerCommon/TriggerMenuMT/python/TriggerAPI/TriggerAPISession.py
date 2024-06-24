#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__author__  = 'Will Buttinger'
__version__="$Revision: 1.0 $"
__doc__="Provides a helper class for managing a session of interactions with the TriggerAPI singleton"

from TriggerMenuMT.TriggerAPI import SerializeAPI
from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod,TriggerType
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class TriggerAPISession:
    """
    --------------------------------------------------------------------------------------------------------------------
    TriggerAPI helper class. Use the following import in your code:

      from TriggerMenuMT.TriggerAPI import TriggerAPISession,TriggerType,TriggerPeriod

    Examples of use:
    ================

    Set of triggers of a given type that are unprescaled for an entire GRL:

      s = TriggerAPISession("path/to/grl.xml") # can be a PathResolver path as well
      triggers = s.getLowestUnprescaled(triggerType=TriggerType.el_single)

    Dictionary of sets of triggers of a given type that are unprescaled, for each run in the GRL:

      s = TriggerAPISession("path/to/grl.xml")
      triggersByRun = s.getLowestUnprescaledByRun(triggerType=TriggerType.el_single)

    Set of triggers that are unprescaled for all runs between two run numbers (inclusive), in a GRL:

      s = TriggerAPISession("path/to/grl.xml")
      triggers = s.getLowestUnprescaledByRun(triggerType=TriggerType.el_single,runStart=123456,runEnd=234567)

    Other helpful methods are:

      - Set of runs present in the session's GRL: s.runs()
      - List of trigger types: [x.name for x in TriggerType]
      - Dictionary of livefractions between given runs, key = trigger chain name:
        liveFractions = s.getLiveFractions(triggerType=TriggerType.el_single,runStart=123456,runEnd=234567)
      - Dictionary of chains (key is chain.name): s.chains()
      - Set of triggers that are deemed to be of same type and lower threshold than a given trigger and unprescaled:
        triggers = s.getLowerPrescaled(chainName="myChain")

    Each method accepts an "additionalTriggerType" parameter that is used for multi-leg triggers of different type
    (e.g. e-mu triggers).

    Instead of passing a GRL you can pass a menu name ("menu_name") in the constructor, and the unprescaled
    triggers will be the Primary|TagAndProbe triggers from the menu.


    Saving a session
    ================
    Sessions can be saved to json file and reloaded at a later time (to save requerying the database):

        s.save("myDump.json")
        s2 = TriggerAPISession(json="myDump.json") # reloads the session

    --------------------------------------------------------------------------------------------------------------------
    """


    def __init__(self, input=None, *, grl=None, flags=None, json=None, menu=None, period=None):
        """
        Specify one and only one of the following parameters to construct your API session:

        :param input: If specified, will try to auto-infer which of the things below it is:

        :param grl: Path to a GRL file, locatable by PathResolver
        :param flags: flag container, used if reading triggers from the trigger menu
        :param json: Path to a JSON file, locatable by PathResolver, containing a cache of TriggerAPI session
        :param menu: Specify a menu to use, such as "Physics_pp_run3_v1". This is otherwise taken from flags
        :param period: Legacy option, can specify a TriggerPeriod and will load through the hardcoded GRLs (TriggerPeriodData)
        """

        if input is not None:
            if type(input)==str:
                if input.endswith(".xml"):
                    log.info("Loading session for GRL:" + input)
                    grl = input
                elif input.endswith(".json"):
                    log.info("Loading saved session from:" + input)
                    json = input
                else:
                    log.info("Loading session for menu:" + input)
                    menu = input
            else:
                raise RuntimeError("Unsupported input type:" + type(input).__name__)


        # the following represents the complete "state" of the TriggerAPI
        self.dbQueries = {}
        self.customGRL = None
        self.flags     = None
        self.release   = None
        self.cacheread = True # always prevent auto-loading of cache in singleton

        if json is not None:
            self.dbQueries = SerializeAPI.load(json)
        elif grl is not None:
            from PathResolver import PathResolver
            grl = PathResolver.FindCalibFile(grl)
            self.customGRL = grl
        elif flags is not None:
            self.flags = flags
        elif menu is not None:
            from AthenaConfiguration.AllConfigFlags import initConfigFlags
            self.flags = initConfigFlags()
            self.flags.Trigger.triggerMenuSetup = menu
            self.flags.lock()
        elif period is not None:
            TriggerAPI.reset()
            TriggerAPI._loadTriggerPeriod(period,reparse=False)
            if not TriggerAPI.dbQueries:
                raise RuntimeError("Failed to load TriggerAPI information for period")
            import copy
            self.dbQueries = copy.deepcopy(TriggerAPI.dbQueries)
        else:
            raise RuntimeError("Must specify one of: grl, flags, json, menu, period")

        if self.flags is not None or self.customGRL is not None:
            TriggerAPI.reset()
            period = TriggerPeriod.future2e34 # used when loading with a flags container
            if self.flags is not None:
                TriggerAPI.setConfigFlags(self.flags)
            else:
                TriggerAPI.setCustomGRL(self.customGRL)
                period = TriggerPeriod.customGRL
            TriggerAPI._loadTriggerPeriod(period,reparse=False)
            if not TriggerAPI.dbQueries:
                raise RuntimeError("Failed to load TriggerAPI information")
            import copy
            self.dbQueries = copy.deepcopy(TriggerAPI.dbQueries)

        # TODO:
        # for any query loaded with an actual period enum (so through json or period arg)
        # we should use TriggerPeriodData to assign per run values of activeLB and totalLB
        # could then merge into a single ti object ... but then need to look at is2015 in isLowerThan
        # since there is special behaviour for 2015 that will be lost

        pass

    def save(self, path):
        """
        :param path: Save a cache of the current session to the given json file
        :return: result of json dump
        """
        return SerializeAPI.dump(self.dbQueries,path)


    def chains(self,*,triggerType=TriggerType.ALL):
        """
        :param triggerType: you can list available types with "[x.name for x in TriggerType]"
        :return: dictionary of triggerChain objects of given types, key = chain Name
        """
        if len(self.dbQueries)>1:
            raise RuntimeError("Unsupported in multi-period TriggerAPI sessions (should only happen if using a period enum or an old json cache)")

        if not isinstance(triggerType,list): triggerType = [triggerType,TriggerType.UNDEFINED]
        if len(triggerType)==1: triggerType += [TriggerType.UNDEFINED]
        elif len(triggerType) > 2:
            raise RuntimeError("More than two trigger types not currently supported")

        out = {}
        for tc in self.triggerInfo().triggerChains:
            if not tc.passType(triggerType[0],triggerType[1]): continue
            out[tc.name] = tc
        return out

    def triggerInfo(self):
        return self.dbQueries[list(self.dbQueries.keys())[0]]

    def runs(self):
        """
        :return: set of runs covered by this session
        """
        out = set()
        for ti in self.dbQueries.values():
            for tc in ti.triggerChains:
                for run in tc.activeLBByRun.keys():
                    out.add(run)
        return out

    def setRunRange(self,start=0,end=999999):
        for ti in self.dbQueries.values():
            ti.setRunRange(start,end)

    def getLowestUnprescaled(self,*, triggerType=TriggerType.ALL,livefraction=1.0,runStart=0,runEnd=999999):
        """
        :param triggerType: list available types with "[x.name for x in TriggerType] .. provide a list of length 2 for multi-leg types"
        :param livefraction: threshold to be considered unprescaled
        :param runStart:
        :param runEnd:
        :return: set of lowest unprescaled (according to livefraction) triggers of given type
        """

        
        if not isinstance(triggerType,list): triggerType = [triggerType,TriggerType.UNDEFINED]
        if len(triggerType)==1: triggerType += [TriggerType.UNDEFINED]
        elif len(triggerType) > 2:
            raise RuntimeError("More than two trigger types not currently supported")

        self.setRunRange(runStart,runEnd)
        out = set()
        for ti in self.dbQueries.values():
            out.update(ti._getLowestUnprescaled(triggerType[0], triggerType[1], "", livefraction))
        self.setRunRange() # reset to include all ranges

        if not out and livefraction==1.0 and list(self.dbQueries.keys())[0][1]:
            log.warning("No triggers found that are fully unprescaled in your GRL ... checking for livefractions per run:")
            # check result by-run to see if there are problems with individual runs (possibly lumiblocks included in each)
            for run in sorted(list(self.runs())):
                liveFractions = self.getLiveFractions(triggerType=triggerType,runStart=run,runEnd=run)
                lf = max(liveFractions.values())
                if lf < 1 and lf > 0.9:
                    log.warning(f"run {run} has maximum livefraction {lf} - prescaled LBs may have been included in your GRL accidentally. Please report this to Data Preparation")
                elif lf==1.0:
                    log.info(f"run {run} is unprescaled")

        return out

    def getLowestUnprescaledByRun(self,*,triggerType=TriggerType.ALL,livefraction=1.0,runStart=0,runEnd=999999):
        """

        :param triggerType:
        :param livefraction:
        :param runStart:
        :param runEnd:
        :return: lowest unprescaled trigger by run. If this session does not have per-run info, all triggers will be listed under a dummy key of ""
        """
        if not self.runs(): # case where loaded from trigger menu, for example
            return {"":self.getLowestUnprescaled(triggerType=triggerType,livefraction=livefraction,runStart=runStart,runEnd=runEnd)}
        out = {}
        for run in self.runs():
            if int(run)<runStart or int(run)>runEnd: continue
            out[run] = self.getLowestUnprescaled(triggerType=triggerType,livefraction=livefraction,runStart=run,runEnd=run)
        return out

    def getLowestUnprescaledAnyRun(self,*,triggerType=TriggerType.ALL,livefraction=1.0,runStart=0,runEnd=999999):
        out = set()
        for tc in self.getLowestUnprescaledByRun(triggerType=triggerType,livefraction=livefraction,runStart=runStart,runEnd=runEnd).values():
            out.update(tc)
        return out
    def getLiveFractions(self,*,triggerType=TriggerType.ALL,runStart=0,runEnd=999999):
        """
        :param triggerType: can be a single type or a list of types
        :param runStart:
        :param runEnd:
        :return: a dictionary of live fractions for triggers matching given trigger types
        """
        out = {}
        self.setRunRange(runStart,runEnd)
        for x in self.chains(triggerType=triggerType).values():
            out[x.name] = x.livefraction
        self.setRunRange()
        return out

    def getLowerUnprescaled(self,*,chainName,triggerType=TriggerType.ALL,livefraction=1.0,runStart=0,runEnd=999999):
        """
        :param chainName:
        :param triggerType:
        :param livefraction:
        :param runStart:
        :param runEnd:
        :return: set of chains of unprescaled triggers that were lower than the given chain
        """

        chains = self.chains()
        if chainName not in chains:
            raise RuntimeError(chainName + " not found")
        chain = chains[chainName]
        self.setRunRange(runStart,runEnd)
        out = set()
        for x in self.chains(triggerType=triggerType).values():
            if x.name==chain.name: continue
            if not x.isUnprescaled(livefraction): continue
            if x.isLowerThan(chain,period=self.triggerInfo().period)==1: out.add(x)
        self.setRunRange()
        return out


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        prog='tapis',
        description="Example usage: tapis path/to/grl.xml getLowestUnprescaled --triggerType el_single",
        epilog='General structure is: tapis [constructor arg] [command] [--commandOpt1] [--commandOpt2] ...',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("--save",default=None,help="If specified, the path to save the session to as a json file")

    parser.add_argument("input",metavar="grl/menu/json",help="Either a GRL, a menu name, or a json session cache file. PathResolve paths supported")
    subparsers = parser.add_subparsers(help="Available subcommands",dest="command")


    parser_getLowestUnprescaled = subparsers.add_parser('getLowestUnprescaled',help='get lowest unprescaled chain names')
    parser_getLowestUnprescaled.add_argument("--livefraction",type=float,default=1.0,help="EXPERT OPTION: lower the livefraction threshold for trigger to be considered unprescaled")

    parser_chains = subparsers.add_parser('chains',help='Show info about a chain or selection of chains')
    parser_chains.add_argument('--chainName',type=str,help="name of chain or wildcarded string",default="*",nargs='?')

    parser_runs = subparsers.add_parser('runs',help='List runs available in the session')

    parser_getLowerUnprescaled = subparsers.add_parser('getLowerUnprescaled',help='get chains that are deemed to be of same type but lower and also unprescaled compared to a given chain')
    parser_getLowerUnprescaled.add_argument('chainName',type=str,help="name of chain")
    parser_getLowerUnprescaled.add_argument("--livefraction",type=float,default=1.0,help="EXPERT OPTION: lower the livefraction threshold for trigger to be considered unprescaled")


    for p in [parser_getLowestUnprescaled,parser_chains,parser_runs,parser_getLowerUnprescaled]:
        #p.add_argument("input",metavar="grl/menu/json",help="Either a GRL, a menu name, or a json session cache file. PathResolve paths supported")
        p.add_argument("--triggerType",choices=[x.name for x in TriggerType],nargs='+',default=["ALL"],help="can specify up to two trigger types")
        p.add_argument("--runStart",type=int,default=0,help="First runNumber to consider")
        p.add_argument("--runEnd",type=int,default=999999,help="Last runNumber to consider")



    args = parser.parse_args()

    if args.command is None: args.command = "getLowestUnprescaled"

    s = TriggerAPISession(args.input)
    if args.save: s.save(args.save)

    # convert triggerTypes into required enums
    if "triggerType" in args:
        args.triggerType = [TriggerType[t] for t in args.triggerType]

    pandasPrint=False

    if args.command == "getLowestUnprescaled":
        result = s.getLowestUnprescaled(triggerType=args.triggerType,
                                    livefraction=args.livefraction,
                                    runStart=args.runStart,runEnd=args.runEnd)
        s.setRunRange(args.runStart,args.runEnd)
        chains = s.chains(triggerType=args.triggerType)
        result = [{"name":chains[c].name,"triggerType":TriggerType.toStr(chains[c].triggerType),"livefraction":chains[c].livefraction} for c in result]
        pandasPrint=True
    elif args.command == "chains":
        s.setRunRange(args.runStart,args.runEnd)
        import fnmatch
        result = {k: v for k,v in s.chains(triggerType=args.triggerType).items() if fnmatch.fnmatch(k,args.chainName)}
        result = [{"name":c.name,"triggerType":TriggerType.toStr(c.triggerType),"livefraction":c.livefraction} for c in result.values()]
        pandasPrint = True
    elif args.command == "runs":
        s.setRunRange(args.runStart,args.runEnd)
        result = sorted(list(s.runs()))
    elif args.command == "getLowerUnprescaled":
        result = s.getLowerUnprescaled(chainName=args.chainName,triggerType=args.triggerType,livefraction=args.livefraction,runStart=args.runStart,runEnd=args.runEnd)
        result = [{"name":c.name,"triggerType":TriggerType.toStr(c.triggerType),"livefraction":c.livefraction} for c in result]
        pandasPrint=True
    if pandasPrint:
        import pandas as pd
        #pd.options.display.max_colwidth = None
        df = pd.DataFrame(result)
        print(df.sort_values(by=['triggerType','livefraction','name'],ascending=[True,False,True]).to_string(index=False)) # noqa: ATL901
        result = None # so we don't print again below

    if result is not None:
        import pprint
        pprint.pp(result)

