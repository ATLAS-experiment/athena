# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations
from typing import TYPE_CHECKING

import re
from importlib import import_module
from collections import defaultdict as ddict
from pathlib import Path
from typing import Any, Iterator

from AthenaCommon.Logging import logging
from PyUtils.moduleExists import moduleExists

from .Base.GlobalAlgorithm import GlobalMultiplicityAlgo
from .Base.Items import MenuItem
from .Base.L1Menu2JSON import L1MenuJSONConverter
from .Base.L1MenuFlags import L1MenuFlags
from .Base.L1MenuRun4 import MenuMetaInfo, L1MenuRun4
from .Base.Limits import Limits
from .Base.Thresholds import GlobalThreshold, Threshold
from .Config.TriggerTypeDef import TT
from .Config.TopoAlgoDefMultiplicity import TopoAlgoDefMultiplicity
from .Config.ItemDef import ItemDef
from .Config.ItemDefRun4 import ItemDef as ItemDefRun4
from .Menu.MenuMapping import menuMap


if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    from .Base.GlobalAlgorithm import GlobalAlgorithm

log = logging.getLogger(__name__)

"""
L1MenuConfig is responsible for building the L1 Menu

Building a menu happens in two stages

1) First all configuration entities are loaded. These are defined in the directory `TriggerMenuMT/Config/` in *Def.py files
  * Items are definded in Config/ItemDefRun4.py
  * Thresholds are defined in files ThresholdDefRun4.py
  * L1Calo configurations will be defined in Config/L1CaloDefRun4.py
  * Global algorithms (all ACCs) are defined in files GlobalACCDefRun4.py
  * Global hypothesis algorithms are defined in two separate files
    - multiplicity counting algorithms are defined in GlobalHypoMultiplicityAlgDefRun4.py
    - topological selection algorithms are defined in GlobalHypoTopologicalAlgDefRun4.py
2) Second the menu is assembled. This assembly has two parts:
  a) the low-level menu assembly, which includes the configurations of L1Calo, MUCTPI, and L1Global,
     and also the configuration of the connection between L1Global and CTP.
  b) the high-level menu assembly, which composes the list of L1 trigger items
  At the end of each of these two assembly steps it is cross-checked if the desired menu can be fulfilled by the
  hardware and current firmware
"""


class L1MenuConfigRun4:

    def __init__(self, flags: AthConfigFlags, inputFile: Path | None = None):

        self.menuFullName: str = flags.Trigger.triggerMenuSetup
        self.menuInfo: MenuMetaInfo = self._menuResolution(self.menuFullName)
        self.menuName: str = self.menuInfo.menuBaseName
        self.inputFile: Path | None = inputFile
        self.l1menuFromFile: bool = (self.inputFile is not None)
        self.isMenuGenerated: bool = False

        L1MenuFlags.MenuSetup = self.menuFullName
        L1MenuFlags.Run = 4
        L1MenuFlags.CTPVersion = 4

        # menu
        self.l1menu = L1MenuRun4(menuInfo=self.menuInfo, flags=flags)

        if not self._checkMenuExistence():
            log.error("Generating L1 menu %s is not possible", self.menuInfo.menuBaseName)
            raise RuntimeError("Generating L1 menu is not possible, menu file does not exist")

        # all registered items
        self._registeredItems: dict[str, MenuItem] = {}

        # all registered thresholds
        self._registeredThresholds: dict[str, Threshold] = {}

        # all registered global algos
        self._registeredGlobalAlgos: dict[str, Any] = {}

        # threshold statistics
        self._registeredThresholdsStats: dict[str, int] = {
            "muon": 0,
            "calo": 0,
            "nim": 0,
            "global": 0
        }

        self._generate(flags)

    @property
    def run(self):
        return 4

    def _generate(self, flags: AthConfigFlags):

        log.info("=== Generating L1 menu %s ===", self.menuInfo.menuBaseName)

        log.info("=== Registering definitions of algorithms, thresholds, and items ===")

        self._registerAvailableItemsThresholdsAlgorithms()

        log.info("=== Loading definition of menu ===")

        # Build menu

        # puts thresholds and items names for the menu into the LVL1Flags
        self._importMenuDefinition()

        self._extendInformation()

        self._generateGlobalHypoMenu()

        self._generateMenu(flags)

        self.isMenuGenerated = True

    @staticmethod
    def _menuResolution(fullMenuName: str) -> MenuMetaInfo:
        """Resolve the menu name to the actual menu to be used based from the mapping above.
        """

        # remove prescale suffix
        menuBaseName = fullMenuName
        if fullMenuName.endswith('prescale'):
            # take everything upto _vXX
            if (m := re.match(r'\w*_v\d*', fullMenuName)) is not None:
                menuBaseName = m.group(0)

        if (menuInfo := menuMap.get(menuBaseName)) is not None:
            menuFile, menuInputFile, _ = (menuInfo + ["", ""])[:3]
        else:
            menuFile = menuBaseName
            menuInputFile = menuFile + "_inputs"

        menuPurpose = MenuMetaInfo.MenuType.PHYSICS
        if "MC_" in menuBaseName:
            menuPurpose = MenuMetaInfo.MenuType.MC
        elif "Dev_" in menuBaseName:
            menuPurpose = MenuMetaInfo.MenuType.DEV
        elif "HI_" in menuBaseName:
            menuPurpose = MenuMetaInfo.MenuType.HI
        elif "AllCTPIn_" in menuBaseName:
            menuPurpose = MenuMetaInfo.MenuType.ALLCTPIN

        # for run 4 fall back to default menu during LS3 development period
        moduleName = 'TriggerMenuMT.L1.Menu.Menu_%s' % menuFile
        if not moduleExists(moduleName):
            log.info("Could not find menu definition for %s (module %s does not exist)", fullMenuName, moduleName)
            raise ModuleNotFoundError(f"Could not find menu definition for {fullMenuName}")

        return MenuMetaInfo(fullMenuName, menuBaseName=menuBaseName,
                            menuFile=menuFile, menuInputFile=menuInputFile,
                            menuPurpose=menuPurpose)

    def thresholdExists(self, thrName: str) -> bool:
        return thrName in self._registeredThresholds

    def getDefinedThreshold(self, name: str) -> Threshold | None:
        return self._registeredThresholds.get(name)

    def getDefinedThresholds(self) -> Iterator[Threshold]:
        return iter(self._registeredThresholds.values())

    ###
    # registration of available components
    ###

    # global algorithms
    def registerGlobalAlgo(self, algo: GlobalAlgorithm):
        """Register a L1Global algorithm for later use"""

        if algo.name in self._registeredGlobalAlgos:
            raise RuntimeError(f'Global algo {algo.name} is already registered ')
        self._registeredGlobalAlgos[algo.name] = algo
        log.debug("Registered global algo: %s", algo.name)
        return algo

    def registerThreshold(self, thr: Threshold):
        """
        Add externally created L1 threshold to the list of available thresholds.
        """
        if self.thresholdExists(thr.name):
            raise RuntimeError("LVL1 threshold of name '%s' already defined, need to abort" % thr.name)

        self._registeredThresholds[thr.name] = thr

        # collect stats
        isTopo = isinstance(thr, GlobalThreshold)
        if isTopo:
            self._registeredThresholdsStats["global"] += 1
            return

        isNIM = thr.ttype in ['NIM', 'CALREQ', 'MBTSSI', 'MBTS', 'LUCID', 'BCM', 'BCMCMB', 'ZDC', 'BPTX', 'ZB']
        if isNIM:
            self._registeredThresholdsStats["nim"] += 1
        elif thr.ttype == 'MU':
            self._registeredThresholdsStats["muon"] += 1
        else:
            self._registeredThresholdsStats["calo"] += 1

    def _registerGlobalOutputsAsThresholds(self):
        """
        Register all L1Global triggers that are available as allowed input to the menu
        """

        # for all global algorithms the outputs (sometimes multiple) are defined as thresholds
        outputLines = []
        for algo in self._registeredGlobalAlgos.values():
            if isinstance(algo, GlobalMultiplicityAlgo):
                continue  # multiplicity counters are already registered as thresholds
            outputLines += algo.output() if (isinstance(algo.output(), list)) else [algo.output()]
        _globalTriggers = sorted(outputLines)
        log.info("... found %i global triggerlines", len(_globalTriggers))
        log.debug("%r", _globalTriggers)

        multibitPattern = re.compile(r"(?P<line>.*)\[(?P<bit>\d+)\]")
        multibitTopoTriggers = set()
        for topoLineName in _globalTriggers:
            m = multibitPattern.match(topoLineName)  # tries to match "trigger[bit]"
            if m:
                topoThrName = m.groupdict()['line']
                multibitTopoTriggers.add(topoThrName)
            else:
                topoThrName = topoLineName
                GlobalThreshold(name=topoThrName)
        # create thresholds from topo-multibit
        for topoThrName in multibitTopoTriggers:  # ( 'MULT-CMU4ab', ...)
            GlobalThreshold(name=topoThrName)

    # Items
    def registerItem(self, name: str, item: MenuItem):
        """ Adds a LVL1 item to the set of items which are registered for further use"""
        if name in self._registeredItems:
            log.error('LVL1 item %s is already registered with ctpid=%d',
                      name, int(self._registeredItems[name].ctpid))
        else:
            self._registeredItems[name] = item

    def getRegisteredItem(self, name: str) -> MenuItem | None:
        return self._registeredItems.get(name)

    def writeJSON(self, outputFile, bgsOutputFile=None, destdir: Path = Path("./")):
        if self.isMenuGenerated:
            outputFile = destdir / outputFile
            if bgsOutputFile is not None:
                bgsOutputFile = destdir / bgsOutputFile
            L1MenuJSONConverter(l1menu=self.l1menu, outputFile=outputFile,
                                bgsOutputFile=bgsOutputFile).writeJSON(pretty=True)
            return outputFile, bgsOutputFile
        else:
            log.error("No menu was generated, can not create json file")
            return None

    def _checkMenuExistence(self) -> bool:
        """Checks if menu file can exists"""
        modname = f'TriggerMenuMT.L1.Menu.Menu_{self.menuInfo.menuFile}'
        if not moduleExists(modname):
            log.error("No L1 menu available for %s, module %s does not exist", self.menuInfo.menuBaseName, modname)
            return False

        return True

    def _importMenuDefinition(self):
        """
        Defines the list if item and threshold names that will be in the menu
        Calls defineMenu() of the correct module 'Menu_<menuname>.py'
        Menu.defineMenu() defines the menu via L1MenuFlags "items", "thresholds",
        """

        # we apply a hack here. menu group is working on Dev_pp_run3_v1, until ready we will use MC_pp_run3_v1
        log.info("Reading TriggerMenuMT.Menu.Menu_%s", self.menuInfo.menuFile)
        menumodule = __import__('TriggerMenuMT.L1.Menu.Menu_%s' % self.menuInfo.menuFile, globals(), locals(), ['defineMenu'], 0)
        menumodule.defineMenu()
        log.info("... L1 menu '%s' contains %i items", self.menuInfo.menuFile, len(L1MenuFlags.items()))
        if log.getEffectiveLevel() <= logging.DEBUG:
            log.debug("Items: %s", ', '.join(L1MenuFlags.items()))

        log.info("Importing input definition from TriggerMenuMT.L1.Menu.Menu_%s", self.menuInfo.menuInputFile)
        topomenumodule = __import__('TriggerMenuMT.L1.Menu.Menu_%s' % self.menuInfo.menuInputFile, globals(), locals(), ['defineMenu'], 0)
        topomenumodule.defineInputsMenu()  # this adds the inputs definition (boards) to L1MenuFlags.boards
        connectorCount = 0
        algoCount = 0
        for _, boardDef in L1MenuFlags.boards().items():
            if "connectors" in boardDef:
                connectorCount += len(boardDef["connectors"])
                for c in boardDef["connectors"]:
                    if "thresholds" in c:
                        algoCount += len(c["thresholds"])
                    elif "algorithmGroups" in c:
                        for t in c["algorithmGroups"]:
                            algoCount += len(t["algorithms"])
                    else:
                        for t in c["signalGroups"]:
                            algoCount += len(t["signals"])
        log.info("    with %i connectors and %i input signals", connectorCount, algoCount)

    def _registerAvailableItemsThresholdsAlgorithms(self):
        """
        Registers the list if items and thresholds that could be used in the menu
        Calls registerItem() of the correct module 'ItemDef.py'
        """
        MenuItem.setMenuConfig(self)  # from now on all newly created MenuItems are automatically registered here
        Threshold.setMenuConfig(self)  # from now on all newly created Thresholds definitions are automatically registered here

        # register Global hypothesis algorithms
        defFile = "TriggerMenuMT.L1.Config.GlobalAlgoDef"
        log.info("Importing %s", defFile)
        try:
            import_module(defFile).__getattribute__('GlobalAlgoDef').registerGlobalAlgos(self)
        except ImportError as ie:
            log.fatal("No global algorithm definition file found. %s", ie.msg)
            raise
        log.info("... registered %i global algorithms for the L1Global",
                 len(self._registeredGlobalAlgos))

        log.info("Importing TriggerMenuMT.Config.ThresholdDefRun4")
        from .Config.ThresholdDefRun4 import ThresholdDef
        ThresholdDef.registerThresholds(self, self.menuFullName)

        log.info("Turning global algorithm outputs into thresholds (except multiplicity counters)")
        self._registerGlobalOutputsAsThresholds()

        self._printRegisteredThresholdInfo()

        log.info("Registering items from TriggerMenuMT.Config.ItemDefRun4")
        ItemDefRun4.registerItems(self, self.menuInfo)
        log.info("... registered %i defined items", len(self._registeredItems))

    def _printRegisteredThresholdInfo(self):
        thresholdsByType = ddict(list)
        for thr in self._registeredThresholds.values():
            thresholdsByType[thr.ttype].append(thr)

        for ttype, thrList in thresholdsByType.items():
            log.debug("Registered %i thresholds of type %s: %s", len(thrList), ttype, ', '.join([t.name for t in thrList]))
        log.info("... registered %i calo thresholds", self._registeredThresholdsStats["calo"])
        log.info("... registered %i muon thresholds", self._registeredThresholdsStats["muon"])
        log.info("... registered %i nim thresholds", self._registeredThresholdsStats["nim"])
        log.info("... registered %i global hypo thresholds", self._registeredThresholdsStats["global"])

    def _getGlobalAlgo(self, algoName: str):
        if algoName in self._registeredGlobalAlgos:
            return self._registeredGlobalAlgos[algoName]
        msg = f"Global algorithm of name {algoName} is not registered. Please add the algorithm to L1/Config/GlobalAlgoDef.py"
        log.error(msg)
        log.info("Available algorithms in this category are %s", ",".join(self._registeredGlobalAlgos.keys()))
        raise RuntimeError(msg)

    def _extendInformation(self):
        """
        this function is called first after the menu definition is imported
        it can be used to update some of the information or to perform initial checks
        """
        allBoards = (list(L1MenuFlags.boards().items()) + list(L1MenuFlags.legacyBoards().items()))
        # set boardName in each connector
        for (boardName, boardDef) in allBoards:
            boardDef["name"] = boardName
            if "connectors" in boardDef:
                for connDef in boardDef["connectors"]:
                    connDef["board"] = boardName
            else:
                for connDef in boardDef["inputConnectors"]:
                    connDef["board"] = boardName

    def _generateGlobalHypoMenu(self):
        log.info("Generating GlobalHypo menu")
        boardDef = L1MenuFlags.boards()["Global"]
        connDef = boardDef["connectors"][0]
        for thr in connDef['thresholds']:
            if isinstance(thr, tuple):
                thrName = thr[0]
            else:
                thrName = thr
            thrDef = self.getDefinedThreshold(thrName)
            if thrDef is None:
                msg = (
                    f'Threshold {thrName} is required by L1Global to provide output to CTP, specified in connector {connDef["name"]} '
                    f'in file L1/Menu/Menu_{self.menuInfo.menuInputFile}.py), but it is not defined. '
                    f'Please add the threshold {thrName.split("_", 1)[-1]} to L1/Config/ThresholdDefRun4.py or L1/Config/GlobalHypoAlgoDef.py.'
                )
                log.error(msg)
                raise RuntimeError(msg)

            self.l1menu.addThreshold(thrDef)

            algoName = "Mult_" + thrName
            algo = self._getGlobalAlgo(algoName)
            self.l1menu.addGlobalAlgo(algo)

    def _generateGlobalMenu(self):

        boardName = "Global"
        boardDef: dict[str, Any] = L1MenuFlags.boards().get(boardName, {})

        #
        # Add the global thresholds to the menu
        #
        nAlgos = 0
        nLines = 0
        for connDef in boardDef["connectors"]:
            for algGrp in connDef["algorithmGroups"]:
                for topodef in algGrp["algorithms"]:
                    nAlgos += 1
                    nLines += len(topodef.outputlines)
                    # this is identical to outputlines except that ["MULT-CMU4ab[0]", "MULT-CMU4ab[1]"] becomes [ "MULT-CMU4ab" ]
                    outputlines = list(dict.fromkeys(map(lambda x: re.split(r"\[\d\]", x)[0], topodef.outputlines)))
                    for lineName in outputlines:
                        thrName = lineName
                        thr = self.getDefinedThreshold(thrName)  # threshold has to be registered
                        if thr is None:
                            msg = 'Global threshold %s is required in Global2CTP connector %s (file L1/Menu/Menu_%s.py), but it is not registered. ' % (thrName, connDef['name'], self.menuInfo.menuInputFile)
                            msg += 'Please add the threshold output %s to L1/Config/ThresholdDefRun4.py or L1/Config/GlobalHypoAlgoDef.py.' % (thrName.split('_', 1)[-1])
                            log.error(msg)
                            raise RuntimeError(msg)
                        self.l1menu.addThreshold(thr)
        log.info("Generating global menu using %i global algorithms with %i trigger lines", nAlgos, nLines)

        #
        # Add the global algorithms to the menu
        #

        # loop over all topo boards and their connectors
        for connDef in boardDef["connectors"]:

            algoNames = []
            algoNbits = []
            fpgaNames = []
            if connDef["format"] == 'multiplicity':
                for thrName in connDef["thresholds"]:
                    if thrName is None:
                        continue
                    nBits = connDef["nbitsDefault"]
                    if type(thrName) is tuple:
                        (thrName, nBits) = thrName
                    if thrName is None:
                        continue
                    algoname = "Mult_" + thrName
                    algoNames += [algoname]
                    algoNbits += [int(nBits)]
                    fpgaNames += ['']
            elif connDef["format"] == 'topological':
                for algGrp in connDef["algorithmGroups"]:
                    for topodef in algGrp["algorithms"]:
                        algoNames += [topodef.algoname]
                        algoNbits += [-1]
                        fpgaNames += [str(algGrp['fpga'])]
            for algoName, algoBits, fpgaName in zip(algoNames, algoNbits, fpgaNames):
                algo = self._getGlobalAlgo(algoName)
                # check that the output bits of the multiplicity topo algorithms are as many as the output bits of the associated thresholds
                if algoBits > 0:
                    if algoBits != algo.nbits:
                        msg = "Algorithm %s defined with %i bits, but the associated threshold has %i bits " % (algo.name, algo.nbits, algoBits)
                        raise RuntimeError(msg)
                self.l1menu.checkBoardInputs(algo, connDef["name"], fpgaName)
                # add the decision algorithms to the menu
                self.l1menu.addGlobalAlgo(algo)

    def _generateMenu(self, flags):

        if len(self.l1menu.items) > 0:
            log.info("L1MenuConfig.generate() has already been called. Will ignore")
            return

        """
        Generates the menu structure from the list of item and threshold names in the L1MenuFlags
        """

        # ------------------
        # Bunchgroups
        # ------------------
        from .Base.BunchGroupSet import createDefaultBunchGroupSet
        self.l1menu.ctp.bunchGroupSet = createDefaultBunchGroupSet(flags)

        # ------------------
        # Thresholds
        # ------------------
        allBoards = (list(L1MenuFlags.boards().items()) + list(L1MenuFlags.legacyBoards().items()))

        if self.menuInfo.menuPurpose == MenuMetaInfo.MenuType.ALLCTPIN:
            # Only after we know all boards are defined, in the special case
            # that we are running the dummy L1 menu for CTP input configuration
            # we need to register more items that access every input
            ItemDef.registerItems_AllCTPIn(self)

        for (boardName, boardDef) in L1MenuFlags.legacyBoards().items():
            for connDef in boardDef["connectors"]:
                # Verify number and ordering of thresholds on L1Calo boards
                if connDef["type"] == "ctpin" and connDef["format"] == "multiplicity":
                    self.l1menu.checkL1CaloThresholds(connDef["thresholds"], boardName, connDef["name"])

        list_of_undefined_thresholds = []
        # new thresholds
        for (boardName, boardDef) in L1MenuFlags.boards().items():
            for connDef in boardDef["connectors"]:
                if connDef["type"] == "ctpin" or connDef["format"] != "multiplicity":
                    continue
                for thrName in connDef["thresholds"]:
                    if type(thrName) is tuple:
                        (thrName, _) = thrName
                    if (thrName is None) or (thrName in self.l1menu.thresholds):
                        continue
                    threshold = self.getDefinedThreshold(thrName)
                    if threshold is None:
                        log.error('Threshold %s is required in menu on board %s, connector %s, but it is not defined',
                                  thrName, boardName, connDef['name'])
                        list_of_undefined_thresholds += [thrName]
                    else:
                        self.l1menu.addThreshold(threshold)
                try:
                    zbThrName = connDef["zeroBias"]
                    zbThr = self.getDefinedThreshold(zbThrName)
                    if zbThr is None:
                        log.error('Zero bias threshold %s is listed in menu but not defined', zbThrName)
                        list_of_undefined_thresholds += [zbThrName]
                    else:
                        self.l1menu.addThreshold(zbThr)
                except KeyError:
                    pass

        # signals from merger boards
        for (boardName, boardDef) in L1MenuFlags.boards().items():
            for connDef in boardDef["connectors"]:
                if connDef["format"] != "simple":
                    continue
                for sGrp in connDef["signalGroups"]:
                    for thrName in sGrp["signals"]:
                        if type(thrName) is tuple:
                            (thrName, _) = thrName
                        if thrName is None or thrName in self.l1menu.thresholds:
                            continue
                        threshold = self.getDefinedThreshold(thrName)
                        if threshold is None:
                            log.error('Threshold %s is required in menu on board %s, connector %s, but it is not defined',
                                      thrName, boardName, connDef['name'])
                            list_of_undefined_thresholds += [thrName]
                        else:
                            self.l1menu.addThreshold(threshold)

        # ctpin thresholds
        for (_, boardDef) in allBoards:
            for connDef in boardDef["connectors"]:
                if connDef["type"] != "ctpin":
                    continue
                for entry in connDef["thresholds"]:
                    thrNames = []
                    if type(entry) is dict:
                        # section that defines topo legacy thresholds
                        thrNames = sum([x.outputlines for x in entry["algorithms"]], [])
                    elif type(entry) is str:
                        thrNames = [entry]
                    elif type(entry) is tuple:
                        thrNames = [entry[0]]

                    for thrName in thrNames:
                        if thrName is None or thrName in self.l1menu.thresholds:
                            continue
                        threshold = self.getDefinedThreshold(thrName)
                        if threshold is None:
                            log.error('Threshold %s is listed in menu but not defined', thrName)
                            list_of_undefined_thresholds += [thrName]
                        else:
                            self.l1menu.addThreshold(threshold)
                try:
                    zbThrName = connDef["zeroBias"]
                    zbThr = self.getDefinedThreshold(zbThrName)
                    if zbThr is None:
                        log.error('Zero bias threshold %s is listed in menu but not defined', zbThrName)
                        list_of_undefined_thresholds += [zbThrName]
                    else:
                        self.l1menu.addThreshold(zbThr)
                except KeyError:
                    pass

        if len(list_of_undefined_thresholds) > 0:
            raise RuntimeError("Found undefined threshold in menu %s, please add these thresholds to l1menu/ThresholdDef.py: %s" %
                               (self.l1menu.menuName, ', '.join(list_of_undefined_thresholds)))

        # ------------------
        # Items
        # ------------------
        # build list of items for the menu from the list of requested names
        itemsForMenu = []
        ctpIdMap = L1MenuFlags.CtpIdMap()
        for itemName in L1MenuFlags.items():
            registeredItem = self.getRegisteredItem(itemName)
            if registeredItem is None:
                msg = f"L1 item '{itemName}' has not been defined in L1/Config/ItemDef{'Run4' if L1MenuFlags.Run() == 4 else ''}.py"
                log.error(msg)
                raise RuntimeError(msg)

            if itemName in ctpIdMap:
                newCTPID = ctpIdMap[itemName]
                registeredItem.setCtpid(newCTPID)

            for thrName in registeredItem.thresholdNames():
                if thrName not in self.l1menu.thresholds:
                    msg = (
                        f"L1 item {itemName} has been added to the menu L1/Menu/Menu_{self.menuInfo.menuFile}.py, "
                        f"but the required threshold {thrName} is not listed as input in L1/Menu/Menu_{self.menuInfo.menuInputFile}.py"
                    )
                    log.error(msg)
                    raise RuntimeError(msg)

            itemsForMenu += [registeredItem]

        # CTP IDs available for assignment
        assigned_ctpids = set([item.ctpid for item in itemsForMenu])
        available_ctpids = sorted(list(set(range(Limits.MaxTrigItems)) - assigned_ctpids), reverse=True)

        # add the items to the menu
        for item in itemsForMenu:
            # set the physics bit
            if not item.name.startswith('L1_CALREQ'):
                item.setTriggerType(item.trigger_type | TT.phys)
            # assign ctp IDs to items that don't have one
            if item.ctpid == -1:
                if len(available_ctpids) == 0:
                    raise RuntimeError("No more CTP IDs available at L1!!")
                item.setCtpid(available_ctpids.pop())
            # add the items into the menu
            self.l1menu.addItem(item)

        # ------------------
        # Connectors
        # ------------------

        for (_, boardDef) in allBoards:
            for connDef in boardDef["connectors"]:
                self.l1menu.addConnector(connDef)

        # ------------------
        # Boards
        # ------------------

        for (_, boardDef) in allBoards:
            self.l1menu.addBoard(boardDef)

        # ------------------
        # Mark items legacy
        # ------------------
        legacyThresholdsSet = set()  # determine from connectors
        for conn in self.l1menu.connectors:
            if not conn.isLegacy():
                continue
            legacyThresholdsSet.update(conn.triggerThresholds())
        for item in self.l1menu.items:
            item.markLegacy(legacyThresholdsSet)

        # assign mapping to thresholds according to their use in the menu
        self.mapThresholds()

        # ------------------
        # CTP
        # ------------------
        self.l1menu.ctp.checkConnectorAvailability(self.l1menu.connectors, self.menuInfo.menuInputFile)

        # set the ctp monitoring (only now after the menu is defined)
        self.l1menu.setupCTPMonitoring()

        # ------------------
        # final consistency check
        # ------------------

        self.l1menu.check()

        # check that no L1 items are defined with BGRP0 only
        self.l1menu.checkBGRP()

        # check that only the minimal set of legacy and detector thresholds is used
        if 'pp' in self.l1menu.menuName:
            self.l1menu.checkLegacyThresholds()

        # check for the topo multiplicity algorithms and CTP inputs
        # TOPO1
        TopoAlgoDefMultiplicity.checkMultAlgoFWconstraints(self.l1menu)

        # check #number of CTP inputs and outputs <=512
        self.l1menu.checkCountCTPInputsOutput()

        # check #number of inputs on the CTPIN connectors
        self.l1menu.checkCTPINconnectors()

        # check that performance thresholds are not used in the physics L1 menu
        self.l1menu.checkPerfThresholds()

        # check that every item in the menu is on a board connected to CTP
        self.l1menu.checkItemsHaveInputs()

        # check TOPO bits
        self.l1menu.checkTOPObits()

        # check that items essential for detector operations are monitored
        if 'PhysicsP1' in self.menuFullName:
            self.l1menu.checkHFmonitoring()

    def mapThresholds(self):
        """
        Set the correct mapping of thresholds according to the
        order it was given in L1MenuFlags.thresholds list. That list
        is usually defined in the setupMenu function of each menu

        NIM and CALREQ types are not remapped !!
        """

        alreadyUsedMappingNumbers = ddict(set)
        for thr in self.l1menu.thresholds:
            if thr.mapping < 0:
                continue
            alreadyUsedMappingNumbers[thr.ttype].add(thr.mapping)

        nextFreeMapping = ddict(lambda: 0)
        for k in alreadyUsedMappingNumbers:
            nextFreeMapping[k] = 0
        for thr in self.l1menu.thresholds():
            if thr.mapping < 0:
                while nextFreeMapping[thr.ttype] in alreadyUsedMappingNumbers[thr.ttype]:
                    nextFreeMapping[thr.ttype] += 1
                log.debug('Setting mapping of threshold %s as %i', thr, nextFreeMapping[thr.ttype])
                thr.mapping = nextFreeMapping[thr.ttype]
                nextFreeMapping[thr.ttype] += 1

    def updateItemPrescales(self):
        for (it_name, ps) in L1MenuFlags.prescales().items():
            item = self.l1menu.getItem(it_name)
            if item:
                item.prescale = ps
            else:
                log.warning('Cannot find item %s to set the prescale', it_name)

    # remove prescale suffixes
    def _getMenuBaseName(self, menuName):
        pattern = re.compile(r'_v\d+|DC14')
        patternPos = pattern.search(menuName)
        if patternPos:
            menuName = menuName[:patternPos.end()]
        else:
            log.info('Can\'t find pattern to shorten menu name, either non-existent in name or not implemented.')
        return menuName
