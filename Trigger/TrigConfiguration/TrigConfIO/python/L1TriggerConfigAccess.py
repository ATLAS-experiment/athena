# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from functools import reduce

from TrigConfIO.TriggerConfigAccessBase import TriggerConfigAccess, ConfigType
from AthenaCommon.Logging import logging
log = logging.getLogger('L1TriggerConfigAccess.py')

class L1MenuAccess(TriggerConfigAccess):
    """
    this class provides access to the L1Menu
    the methods are self-explanatory for people with knowledge of the configuration
    """
    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", smkey: int = 0, 
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and smkey
        """
        super().__init__(ConfigType.L1MENU, mainkey = "items",
                         jsonString = jsonString, filename = filename, dbalias = dbalias, dbkey = smkey,
                         useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            2: "SELECT L1MT.L1TM_DATA FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.L1_MENU L1MT WHERE L1MT.L1TM_ID=SMT.SMT_L1_MENU_ID AND SMT.SMT_ID=:dbkey", # for new db schema
            1: "SELECT L1MT.L1MT_MENU FROM {schema}.SUPER_MASTER_TABLE SMT, {schema}.L1_MASTER_TABLE L1MT WHERE L1MT.L1MT_ID=SMT.SMT_L1_MASTER_TABLE_ID AND SMT.SMT_ID=:dbkey"  # for current db schema
        })
        self.load()
        if smkey is not None:
            log.info(f"Loaded L1 menu {self.name()} with {len(self)} items from {dbalias} with smk {smkey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded L1 menu {self.name()} with {len(self)} chains from file {filename}")

    def itemNames(self):
        return self["items"].keys()

    def itemsWithCtpid(self ):
        return self.items( includeKeys = ['ctpid'] )

    def items(self, includeKeys = [] ):
        if includeKeys:
            """ reduce returned dictionary """
            items = self["items"]
            return { x : {k : items[x][k] for k in includeKeys if k in items[x]} for x in items }
        else:
            return self["items"]

    def thresholdTypes(self):
        thrTypes = list(self["thresholds"].keys())
        if "legacyCalo" in thrTypes:
            thrTypes.remove("legacyCalo")
            thrTypes += list(self["thresholds"]["legacyCalo"].keys())
        return thrTypes

    def thresholds(self, thresholdType = None, fulldict = False):
        """
        When setting fulldict the full dictionary is returned, else just thresholds
        """
        if thresholdType:
            if thresholdType == "internal":
                return {}
            if thresholdType in self["thresholds"]:
                if fulldict:
                    return self["thresholds"][thresholdType]
                else:
                    return self["thresholds"][thresholdType]["thresholds"]
            if thresholdType in self["thresholds"]["legacyCalo"]:
                return self["thresholds"]["legacyCalo"][thresholdType]["thresholds"]
            raise RuntimeError("Threshold type %s not known in thresholds section of the menu" % thresholdType)
        else:
            thrs = {}
            for thrType in self.thresholdTypes():
                thrs.update( self.thresholds(thrType) )
            return thrs

    def thresholdNames(self, thresholdType = None):
        if thresholdType == "internal":
            return self["thresholds"]["internal"]["names"]
        elif thresholdType is None:
            return list(self.thresholds().keys()) + self.thresholdNames("internal")
        else:
            return list(self.thresholds(thresholdType).keys())

    def thresholdExtraInfo(self, thresholdType):
        if thresholdType in self["thresholds"]:
            thrDef = self["thresholds"][thresholdType]
        elif thresholdType in self["thresholds"]["legacyCalo"]:
            thrDef = self["thresholds"]["legacyCalo"][thresholdType]
        else:
            raise KeyError("Threshold type %s not known in thresholds section of the menu" % thresholdType)
        return {k:thrDef[k] for k in thrDef if k not in ("thresholds", "type")}

    def topoAlgorithmTypes(self):
        return self["topoAlgorithms"].keys()

    def topoAlgorithms(self, topoAlgoType = None):
        if topoAlgoType:
            return self["topoAlgorithms"][topoAlgoType]
        else:
            d = {}
            for x in self["topoAlgorithms"].values():
                for gr in x.values():
                    d.update(gr)
            return d

    def topoAlgorithmNames(self, topoAlgoType = None):
        allAlgs = self.topoAlgorithms(topoAlgoType)
        return allAlgs.keys()


    def boardNames(self):
        return iter(self["boards"])

    def boards(self):
        return self["boards"]

    def board(self, boardName):
        return self["boards"][boardName]

    def connectorNames(self):
        return iter(self["connectors"])

    def connectors(self):
        return self["connectors"]

    def connector(self, connectorName):
        return self["connectors"][connectorName]

    def ctp(self):
        return self["ctp"]

    def ctpInputs(self, inputType):
        """ inputType should be 'optical', 'electrical' or 'ctpin'
        """
        return self["ctp"]["inputs"][inputType]

    def printSummary(self):
        print("L1 menu %s" % self.name())
        print("Number of items: %i" % len(self))
        print("Number of threshold types: %i" % len(self.thresholdTypes()) )
        print("Number of thresholds: %i" % len(self.thresholds()) )
        print("Number of topo algorithms: %i" % len(self.topoAlgorithms()))
        print("Number of boards: %i (%i are legacy boards)" % ( len(self.boards()), sum(["legacy" in b for b in self.boards().values()]) ))
        print("Number of connectors: %i (%i are legacy connetors)" % ( len(self.connectors()), sum(["legacy" in c for c in self.connectors().values()]) ))
        print("CTP has %i optical, %i electrical, and %i CTPIN inputs" % ( len(self.ctpInputs("optical")), len(self.ctpInputs("electrical")), 
                                                                           len(reduce(
                                                                               lambda s1,i: s1.union(self.ctpInputs("ctpin")[f"slot{i}"].values()),
                                                                               [7,8,9], set()) - set([""]) )))




class L1PrescalesSetAccess(TriggerConfigAccess):
    """
    this class provides access to the L1 prescales set
    the methods are self-explanatory for people with knowledge of the configuration
    """
    @staticmethod
    def calcPrescaleFromCut(cut):
        """
        turns cut value (which is what the hardware is configured with), into a float prescale value
        """
        return 0xFFFFFF / ( 0x1000000 - cut )

    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", l1pskey: int = 0, 
                 useCrest: bool = False, crestServer: str = ""):
        """
        accessor needs to be initialized with either a filename or the dbalias and l1pskey
        """
        super().__init__(ConfigType.L1PS, mainkey = "cutValues", jsonString = jsonString, filename = filename, 
                         dbalias = dbalias, dbkey = l1pskey, useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            1: "SELECT L1PS_DATA FROM {schema}.L1_PRESCALE_SET L1PS WHERE L1PS_ID=:dbkey" # for current and new db schema
        })
        self.load()
        if l1pskey is not None:
            log.info(f"Loaded L1 prescales {self.name()} with {len(self)} items from {dbalias} with psk {l1pskey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded L1 prescales {self.name()} with {len(self)} items from file {filename}")


    def itemNames(self):
        return self["cutValues"].keys()
    def cutValues(self):
        return self["cutValues"]
    def cut(self, itemName):
        return self["cutValues"][itemName]["cut"]
    def prescale(self, itemName):
        return L1PrescalesSetAccess.calcPrescaleFromCut( self.cut(itemName) )

    def enabled(self, itemName):
        return self["cutValues"][itemName]["enabled"]

    def printSummary(self):
        print("L1 prescales set %s" % self.name())
        print("Number of prescales: %i" % len(self) )
        print("Number of enabled prescales: %i" % sum(x["enabled"] for x in self["cutValues"].values()) )


class BunchGroupSetAccess(TriggerConfigAccess):
    """
    this class provides access to the L1 bunchgroup set
    the methods are self-explanatory for people with knowledge of the configuration
    """
    def __init__(self, filename: str = "", jsonString: str = "", dbalias: str = "", bgskey: int = 0, 
                 useCrest: bool = False, crestServer: str = ""):
        super().__init__(ConfigType.BGS, mainkey = "bunchGroups",
                         jsonString = jsonString, filename = filename, dbalias = dbalias, dbkey = bgskey,
                         useCrest=useCrest, crestServer=crestServer)
        self.loader.setQuery({
            1: "SELECT L1BGS_DATA FROM {schema}.L1_BUNCH_GROUP_SET BGS WHERE L1BGS_ID=:dbkey" # for current and new db schema
        })
        self.load()
        if bgskey is not None:
            log.info(f"Loaded L1 bunchgroup set {self.name()} with {len(self)} bunchgroups from {dbalias} with bgsk {bgskey}{' using CREST' if useCrest else ''}")
        elif filename is not None:
            log.info(f"Loaded L1 bunchgroup set {self.name()} with {len(self)} bunchgroups from file {filename}")

