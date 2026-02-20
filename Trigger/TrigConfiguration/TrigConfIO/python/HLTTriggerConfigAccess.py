# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from functools import lru_cache, reduce

from TrigConfIO.TriggerConfigAccessBase import TriggerConfigAccess, ConfigType

from AthenaCommon.Logging import logging
log = logging.getLogger('HLTTriggerConfigAccess.py')

class HLTMenuAccess(TriggerConfigAccess):
    """
    this class provides access to the HLT menu
    the methods are self-explanatory for people with knowledge of the configuration
    """
    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", smkey: int = 0,
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and smkey
        """
        super().__init__(ConfigType.HLTMENU, mainkey = "chains",
                         filename = filename, jsonString = jsonString, dbalias = dbalias, dbkey = smkey,
                         useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            2: "SELECT HMT.HTM_DATA FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.HLT_MENU HMT WHERE HMT.HTM_ID=SMT.SMT_HLT_MENU_ID AND SMT.SMT_ID=:dbkey", # for new db schema
            1: "SELECT HMT.HMT_MENU FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.HLT_MASTER_TABLE HMT WHERE HMT.HMT_ID=SMT.SMT_HLT_MASTER_TABLE_ID AND SMT.SMT_ID=:dbkey"  # for current db schema
        })
        self.load()
        if smkey is not None:
            log.info(f"Loaded HLT menu {self.name()} with {len(self)} chains from {dbalias} with smk {smkey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded HLT menu {self.name()} with {len(self)} chains from file {filename}")

    def chainNames(self):
        return self["chains"].keys()

    def chains(self):
        return self["chains"]

    def streams(self):
        return self["streams"]

    def sequencers(self):
        return self["sequencers"]

    def printSummary(self):
        print("HLT menu %s" % self.name())
        print("Number of chains: %i" % len(self.chains()) )
        print("Number of streams: %i" % len(self.streams()) )
        print("Number of sequencers: %i" % len(self.sequencers()) )

    def printDetails(self):
        import pprint
        print("Chains:")
        pprint.pprint(list(self.chains()))
        print("Streams:")
        pprint.pprint(list(self.streams()))
        print("Sequencers:")
        pprint.pprint(list(self.sequencers()))

class HLTPrescalesSetAccess(TriggerConfigAccess):
    """
    this class provides access to the HLT prescales set
    the methods are self-explanatory for people with knowledge of the configuration
    """
    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", hltpskey: int = 0,
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and hlpskey
        """
        super().__init__(ConfigType.HLTPS, mainkey = "prescales",
                         jsonString = jsonString, filename = filename, dbalias = dbalias, dbkey = hltpskey,
                         useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            1: "SELECT HPS_DATA FROM {schema}.HLT_PRESCALE_SET HPS WHERE HPS_ID=:dbkey" # for current and new db schema
        })
        self.load()
        if hltpskey is not None:
            log.info(f"Loaded HLT prescales {self.name()} (size {len(self)}) from {dbalias} with psk {hltpskey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded HLT prescales {self.name()} with {len(self)} chains from file {filename}")

    def prescales(self):
        return self["prescales"]

    def chainNames(self):
        return iter(self)

    def prescale(self, chainName):
        return self["prescales"][chainName]["prescale"]

    def enabled(self, chainName):
        return self["prescales"][chainName]["enabled"]

    def printSummary(self):
        print("HLT prescales set %s" % self.name())
        print("Number of prescales: %i" % len(self) )
        print("Number of enabled prescales: %i" % sum(x["enabled"] for x in self["prescales"].values()) )



class HLTJobOptionsAccess(TriggerConfigAccess):
    """
    this class provides access to the HLT algorithm configuration
    the methods are self-explanatory for people with knowledge of the configuration
    """
    def __init__(self, filename: str = "", dbalias: str = "", smkey: int = 0,
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and smkey
        """
        super().__init__(ConfigType.HLTJO, mainkey = "properties",
                         filename = filename, dbalias = dbalias, dbkey = smkey,
                         useCrest = useCrest, crestServer = crestServer)
        self.loader.setQuery({
            2: "SELECT JO.HJO_DATA FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.HLT_JOBOPTIONS JO WHERE JO.HJO_ID=SMT.SMT_HLT_JOBOPTIONS_ID AND SMT.SMT_ID=:dbkey", # for new db schema
            1: "SELECT JO.JO_CONTENT FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.JO_MASTER_TABLE JO WHERE JO.JO_ID=SMT.SMT_JO_MASTER_TABLE_ID AND SMT.SMT_ID=:dbkey"  # for current db schema
        })
        self.load()
        if smkey is not None:
            log.info(f"Loaded HLT job options {self.name()} with {len(self)} algorithms from {dbalias} with smk {smkey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded HLT job options {self.name()} with {len(self)} chains from file {filename}")

    def algorithms(self):
        return self["properties"]

    def algorithmNames(self):
        return iter(self)

    def properties(self, algName):
        return self["properties"][algName]

    def name(self):
        # job options don't have a name
        return "HLT JobOptions"

        
    def printSummary(self):
        print("Job options")
        print("Number of algorithms: %i" % len(self) )
        print("Number of properties: %i" % sum(len(alg) for alg in self.algorithms().values()) )


class HLTMonitoringAccess(TriggerConfigAccess):
    """
    this class provides access to the HLT monitoring json
    """
    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", smkey: int = 0, monikey: int = 0,
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and hlpskey
        """
        super().__init__(ConfigType.HLTMON, mainkey = "signatures",
                         jsonString = jsonString, filename = filename, dbalias = dbalias, dbkey = smkey if smkey else monikey,
                         useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            7: (
                "SELECT HMG.HMG_DATA FROM {schema}.HLT_MONITORING_GROUPS HMG, {schema}.SUPER_MASTER_TABLE SMT WHERE HMG.HMG_IN_USE=1 "
                "AND SMT.SMT_HLT_MENU_ID = HMG.HMG_HLT_MENU_ID AND SMT.SMT_ID=:dbkey ORDER BY HMG.HMG_ID DESC"
            )
        } if smkey else {
            7: "SELECT HMG.HMG_DATA FROM {schema}.HLT_MONITORING_GROUPS HMG WHERE HMG.HMG_ID=:dbkey"
        })
        self.load()
        if smkey is not None:
            log.info(f"Loaded HLT monitoring {self.name()} with {len(self)} signatures from {dbalias} with smk {smkey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded HLT monitoring {self.name()} with {len(self)} signatures from file {filename}")
    

    def monitoringDict(self):
        """
        return stored monitoring dictionary
        """
        return self["signatures"]


    def monitoredChains(self, signatures="", monLevels="", wildcard=""):
        """
        return list of all monitored shifter chains for given signature and for a given monitoring level

        signatures - monitored signature or list of signatures for which to return the chains
                     empty string means all signatures
        monLevels - levels of monitoring (shifter, t0 (expert), val (validation))
        wildcard - regexp pattern to match the chains' names

        if monitoring level is not defined return all the chains for given signature
        if signature is not defined return all the chains for given monitoring level
        if both monitoring level and signature are not defined, raturn all chains

        return can be filtered by wildcard
        """
        chains = set()

        if signatures=="": # empty string means all signatures
            signatures = set(self)

        # turn input (str,list) into a set of signature names
        if isinstance(signatures, str):
            signatures = set([signatures])
        signatures = set(signatures)

        # warn about requested signatures that don't have a monitoring entry and remove from the request
        noMonAvailable = signatures.difference(self)
        if noMonAvailable:
            log.warning("These monitoring signatures are requested but not available in HLT monitoring: %s", ', '.join(noMonAvailable))
            signatures.intersection_update(self) # ignore non-existing signatures

        # turn input (str,list) into a set of monLevels
        if isinstance(monLevels, str):
            monLevels = set([monLevels])
        monLevels = set(monLevels)

        for signature in signatures:
            for chainName, targets in self["signatures"][signature].items():
                if monLevels.intersection(targets+[""]): # if there is an overlap between requested and configured
                    chains.add(chainName)

        try:
            import re
            r = re.compile(wildcard)
            chains = filter(r.search, chains)
        except re.error as exc:
            log.warning("Wildcard regex: %r is not correct!", exc)

        # Create set first to ensure uniquness of elements
        return list(chains)


    @lru_cache(maxsize=5)
    def monitoringLevels(self, signatures = None):
        """
        return all monitoring levels
        If one ore more signatures are specified, return only monitoring levels for those
        """
        if signatures is None:
            signatures = set(self)
        if isinstance(signatures, str):
            signatures = set([signatures])
        signatures = set(signatures)
        levels = set()
        for signatureName, chains in self["signatures"].items():
            if signatureName in signatures:
                levels = reduce( lambda x, y: x.union(y), chains.values(), levels )
        return levels


    def printSummary(self):
        print("HLT monitoring groups %s" % self.name())
        print("Signatures (%i): %s" % (len(self), ", ".join(self)) )
        print("Monitoring levels (%i): %s" % (len(self.monitoringLevels()), ", ".join(self.monitoringLevels())))
