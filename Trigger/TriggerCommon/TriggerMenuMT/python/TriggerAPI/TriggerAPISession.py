# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__author__  = 'Will Buttinger'
__version__="$Revision: 1.0 $"
__doc__="Provides a helper class for managing a session of interactions with the TriggerAPI singleton"

from TriggerMenuMT.TriggerAPI import SerializeAPI
from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod,TriggerType


class TriggerAPISession:
    """
    --------------------------------------------------------------------------------------------------------------------
    TriggerAPI helper class. Use the following import in your code:

      from TriggerMenuMT.TriggerAPI import TriggerAPISession,TriggerType,TriggerPeriod

    Examples of use:
    ================

    Set of triggers of a given type that are unprescaled for an entire GRL:

      s = TriggerAPISession(grl="path/to/grl.xml") # can be a PathResolver path as well
      triggers = s.getLowestUnprescaled(triggerType=TriggerType.el_single)

    Dictionary of sets of triggers of a given type that are unprescaled, for each run in the GRL:

      s = TriggerAPISession(grl="path/to/grl.xml")
      triggersByRun = s.getLowestUnprescaledByRun(triggerType=TriggerType.el_single)

    Set of triggers that are unprescaled for all runs between two run numbers (inclusive), in a GRL:

      s = TriggerAPISession(grl="path/to/grl.xml")
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

    Instead of passing a GRL you can pass a menu name (menu="menu_name") in the constructor, and the unprescaled
    triggers will be the Primary|TagAndProbe triggers from the menu.


    Saving a session
    ================
    Sessions can be saved to json file and reloaded at a later time (to save requerying the database):

        s.save("myDump.json")
        s2 = TriggerAPISession(json="myDump.json") # reloads the session

    --------------------------------------------------------------------------------------------------------------------
    """


    def __init__(self, *, grl=None, flags=None, json=None, menu=None, period=None):
        """
        Specify one and only one of the following parameters to construct your API session:

        :param grl: Path to a GRL file, locatable by PathResolver
        :param flags: flag container, used if reading triggers from the trigger menu
        :param json: Path to a JSON file, locatable by PathResolver, containing a cache of TriggerAPI session
        :param menu: Specify a menu to use, such as "Physics_pp_run3_v1". This is otherwise taken from flags
        :param period: Legacy option, can specify a TriggerPeriod and will load through the hardcoded GRLs (TriggerPeriodData)
        """

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


    def chains(self,*,triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED):
        """
        :param triggerType: you can list available types with "[x.name for x in TriggerType]"
        :param additionalTriggerType:
        :return: dictionary of triggerChain objects of given types, key = chain Name
        """
        if len(self.dbQueries)>1:
            raise RuntimeError("Unsupported in multi-period TriggerAPI sessions (should only happen if using a period enum or an old json cache)")
        out = {}
        for tc in self.triggerInfo().triggerChains:
            if not tc.passType(triggerType,additionalTriggerType): continue
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

    def getLowestUnprescaled(self,*, triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED,livefraction=1.0,runStart=0,runEnd=999999):
        """
        :param triggerType: primary trigger type. you can list available types with "[x.name for x in TriggerType]"
        :param additionalTriggerType: optional additional trigger type, for multileg triggers
        :param livefraction: threshold to be considered unprescaled
        :param runStart:
        :param runEnd:
        :return: set of lowest unprescaled (according to livefraction) triggers of given type
        """
        self.setRunRange(runStart,runEnd)
        out = set()
        for ti in self.dbQueries.values():
            out.update(ti._getLowestUnprescaled(triggerType, additionalTriggerType, "", livefraction))
        self.setRunRange() # reset to include all ranges
        return out

    def getLowestUnprescaledByRun(self,*,triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED,livefraction=1.0,runStart=0,runEnd=999999):
        """

        :param triggerType:
        :param additionalTriggerType:
        :param livefraction:
        :param runStart:
        :param runEnd:
        :return: lowest unprescaled trigger by run. If this session does not have per-run info, all triggers will be listed under a dummy key of ""
        """
        if not self.runs(): # case where loaded from trigger menu, for example
            return {"":self.getLowestUnprescaled(triggerType=triggerType,additionalTriggerType=additionalTriggerType,livefraction=livefraction,runStart=runStart,runEnd=runEnd)}
        out = {}
        for run in self.runs():
            if int(run)<runStart or int(run)>runEnd: continue
            out[run] = self.getLowestUnprescaled(triggerType=triggerType,additionalTriggerType=additionalTriggerType,livefraction=livefraction,runStart=run,runEnd=run)
        return out

    def getLowestUnprescaledAnyRun(self,*,triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED,livefraction=1.0,runStart=0,runEnd=999999):
        out = set()
        for tc in self.getLowestUnprescaledByRun(triggerType=triggerType,additionalTriggerType=additionalTriggerType,livefraction=livefraction,runStart=runStart,runEnd=runEnd).values():
            out.update(tc)
        return out
    def getLiveFractions(self,*,triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED,runStart=0,runEnd=999999):
        """
        :param triggerType:
        :param additionalTriggerType:
        :param runStart:
        :param runEnd:
        :return: a dictionary of live fractions for triggers matching given trigger types
        """
        out = {}
        self.setRunRange(runStart,runEnd)
        for x in self.chains(triggerType=triggerType, additionalTriggerType=additionalTriggerType).values():
            out[x.name] = x.livefraction
        self.setRunRange()
        return out

    def getLowerUnprescaled(self,*,chainName,triggerType=TriggerType.ALL,additionalTriggerType=TriggerType.UNDEFINED,livefraction=1.0,runStart=0,runEnd=999999):
        """
        :param chainName:
        :param livefraction:
        :param runStart:
        :param runEnd:
        :return: set of chainNames of unprescaled triggers that were lower than the given chain
        """

        chains = self.chains()
        if chainName not in chains:
            raise RuntimeError(chainName + " not found")
        chain = chains[chainName]
        self.setRunRange(runStart,runEnd)
        out = set()
        for x in self.chains(triggerType=triggerType, additionalTriggerType=additionalTriggerType).values():
            if x.name==chain.name: continue
            if not x.isUnprescaled(livefraction): continue
            if x.isLowerThan(chain,period=self.triggerInfo().period)==1: out.add(x.name)
        self.setRunRange()
        return out



