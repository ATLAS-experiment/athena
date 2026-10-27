# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations
from typing import TYPE_CHECKING


from enum import Enum

from .GlobalAlgorithmsCollection import GlobalAlgorithmsCollection
from .CTP import CTP
from .Items import MenuItem, MenuItemsCollection
from .Thresholds import MenuThresholdsCollection, Threshold
from .Boards import MenuBoardsCollection
from .Connectors import MenuConnectorsCollection, CType
from .MenuUtils import get_smk_psk_Name
from .Limits import Limits
from .ThresholdType import ThrType
from .CTPCondition import ThrCondition

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    from ..Base.GlobalAlgorithm import GlobalAlgorithm


class MenuMetaInfo:
    """Class to bundle all L1 menu related infomation into a single object
    (ideally this should be done for L1 and HLT combined, but for now we only do it for L1)
    This information includes run, menuName, menuFile, menuInputFile, menuLegacyInputFile
    and input files.
    """

    class MenuType(Enum):
        """purpose of the menu: Physics, MC, Dev, HI, AllCTPIN"""
        PHYSICS = "Physics"
        MC = "MC"
        DEV = "Dev"
        HI = "HI"
        ALLCTPIN = "AllCTPIn"

    def __init__(self, menuFullName: str, *,
                 menuPurpose: MenuType, menuBaseName: str, menuFile: str,
                 menuInputFile: str, run: int = 4):
        self.menuFullName: str = menuFullName
        self.menuBaseName: str = menuBaseName
        self.menuFile: str = menuFile
        self.menuInputFile: str = menuInputFile
        self.menuPurpose: MenuMetaInfo.MenuType = menuPurpose
        self.run: int = run

    def __str__(self):
        return f"menuName = {self.menuFullName}, menuFile={self.menuFile}, menuInputFile={self.menuInputFile}, menuPurpose={self.menuPurpose}"

    def __repr__(self):
        return f"MenuMetaInfo(menuName={self.menuFullName}, menuFile={self.menuFile}, menuInputFile={self.menuInputFile}, menuPurpose={self.menuPurpose})"


class L1MenuRun4:
    """
    This class holds everything that is needed to define the menu
    """

    def __init__(self, *, menuInfo: MenuMetaInfo, flags: AthConfigFlags):

        self.menuInfo: MenuMetaInfo = menuInfo

        # items in menu
        self.items: MenuItemsCollection = MenuItemsCollection()

        # store cached flags
        self.flags = flags

        # all thresholds that are in menu (new and legacy)
        self.thresholds = MenuThresholdsCollection(flags)

        # all thresholds that are in menu (new and legacy)
        self.globalAlgos = GlobalAlgorithmsCollection()

        # all connectors between legacyCalo, muctpi, topo and the CTPIN/CTPCORE
        self.connectors = MenuConnectorsCollection()

        # board definition
        self.boards = MenuBoardsCollection()

        # CTP Info in the menu
        self.ctp = CTP(4)

        smk_psk_Name: dict[str, str] = get_smk_psk_Name(self.menuName)
        self.items.menuName = smk_psk_Name["smkName"]
        self.items.pssName = smk_psk_Name["pskName"]

    @property
    def menuName(self) -> str:
        """The name of the menu, e.g. Physics_pp_run4"""
        return self.menuInfo.menuBaseName

    @property
    def menuFullName(self) -> str:
        """The full name of the menu, e.g. Physics_pp_run4_bulk"""
        return self.menuInfo.menuFullName

    def addItem(self, item: MenuItem):
        self.items += item

    def getItem(self, name: str) -> MenuItem:
        return self.items[name]

    def addThreshold(self, threshold: Threshold):
        self.thresholds += threshold

    def addGlobalAlgo(self, algo: GlobalAlgorithm):
        self.globalAlgos.addAlgo(algo)

    def addBoard(self, boardDef):
        return self.boards.addBoard(boardDef)

    def addConnector(self, connDef):
        self.connectors.addConnector(connDef)

    def setupCTPMonitoring(self):
        self.ctp.setupMonitoring(self.menuInfo.menuBaseName, self.items, self.thresholds,
                                 self.connectors, self.menuInfo.menuFullName)

    def check(self):
        log.info("Doing L1 Menu checks")
        from collections import defaultdict as dd
        missing = dd(list)
        allThresholds = set([thr.name for thr in self.thresholds])
        allUsedThresholds = set()
        for item in self.items:
            for thrName in item.thresholdNames():
                if 'SPARE' in thrName:
                    raise RuntimeError("CTP input %s is used by %s but SPARE thresholds are not to be used!" % (thrName, item))
                if thrName not in allThresholds:
                    missing[thrName].append(item.name)
                else:
                    allUsedThresholds.add(thrName)

        for thrName in sorted(missing.keys()):
            log.warning("Threshold %s (used by %s) is not defined in the menu", thrName, ",".join(missing[thrName]))

        if len(allThresholds) - len(allUsedThresholds) > 0:
            unusedThresholds = allThresholds.difference(allUsedThresholds)
            log.debug("The following thresholds are unused")
            log.debug("MU: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("MU")]))
            log.debug("EM: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("EM")]))
            log.debug("HA: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("HA")]))
            log.debug("J: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("J")]))
            log.debug("eFEX: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("e")]))
            log.debug("jFEX: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("j")]))
            log.debug("cTAU: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("cTAU")]))
            log.debug("gFEX: %s", ", ".join([thr for thr in unusedThresholds if thr.startswith("g")]))

    def checkLegacyThresholds(self):
        from collections import defaultdict as dd
        from ..Menu.LegacyMenuThresholds import legacyThresholds
        extraThresholds = dd(list)
        for item in self.items:
            for thrName in item.thresholdNames():
                if thrName[:3] == 'ZB_':
                    thrName = thrName[3:]
                if thrName[0] not in ('e', 'j', 'g', 'c') and thrName[:2] not in ["MU"] and "TOPO" not in thrName[:4]:
                    if thrName not in legacyThresholds:
                        extraThresholds[thrName].append(item.name)

        for thrName in sorted(extraThresholds.keys()):
            log.warning("Threshold %s (used by %s) should not be used!", thrName, ",".join(extraThresholds[thrName]))

    def checkPerfThresholds(self):
        if 'MC' not in self.menuInfo.menuBaseName:
            from collections import defaultdict as dd
            perfThresholds = dd(list)
            for item in self.items:
                for thrName in item.thresholdNames():
                    if 'Perf' in thrName:
                        perfThresholds[thrName].append(item.name)
            for thrName in sorted(perfThresholds.keys()):
                raise RuntimeError("Threshold %s (used by %s) should not be used!", thrName, ",".join(perfThresholds[thrName]))

    def checkBoardInputs(self, algo, connDefName, fpgaName):
        if 'MuCTPi' in connDefName or 'Legacy' in connDefName:
            return

        boardName = connDefName + fpgaName

        allowedInputs = {}
        allowedInputs['Topo1Opt0'] = ['MU', 'eEM', 'eTAU',              'gJ',  'gLJ',                      'ZeroBiasA']  # TOPO1A, fiber 1  # noqa: E201
        allowedInputs['Topo1Opt1'] = ['MU', 'eEM', 'eTAU',              'gJ',  'gLJ',                     ]  # TOPO1A, fiber 2  # noqa: E202
        allowedInputs['Topo1Opt2'] = ['MU',        'eTAU', 'cTAU', 'j',               'cXE', 'gXE', 'gTE', 'gMHT', 'gESPRESSO',               'LArSaturation', 'ZeroBiasB']  # TOPO1B, fiber 1
        allowedInputs['Topo1Opt3'] = ['MU',        'eTAU', 'cTAU', 'j',               'cXE', 'gXE', 'gTE', 'gMHT', 'gESPRESSO', 'gRISTRETTO', 'LArSaturation', 'ZeroBiasB']  # TOPO1B, fiber 2
        allowedInputs['Topo2El0'] = ['MU',        'eTAU',         'j',      ]  # TOPO2, FPGA1 # noqa: E202
        allowedInputs['Topo2El1'] = [      'eEM',                 'j',      ]  # TOPO2, FPGA2 # noqa: E201,E202
        allowedInputs['Topo3El0'] = [      'eEM', 'eTAU',         'j',      ]  # noqa: E201,E202 # TOPO3, FPGA1
        allowedInputs['Topo3El1'] = ['MU', 'eEM', 'eTAU',              'g', ]  # TOPO3, FPGA2  noqa: E221

        if boardName not in allowedInputs.keys():
            raise RuntimeError("Connector name %s not found" % boardName)

        if 'Mult_' in algo.name:
            if not (any(x in algo.threshold for x in allowedInputs[boardName])):
                raise RuntimeError("Algorithm %s in board %s with threshold %s not allowed" % (algo.name, boardName, algo.threshold))

        if 'Mult_' not in algo.name:
            for algoInput in algo.inputs:
                if not (any(x in algoInput for x in allowedInputs[boardName])):
                    raise RuntimeError("Algorithm %s in board %s with input %s not allowed" % (algo.name, boardName, algoInput))

    def checkL1CaloThresholds(self, thresholds, boardName, connName):
        fullName = boardName + connName

        allowedInputs = {}
        allowedInputs['Ctpin7EM1'] = 8 * ['EM']
        allowedInputs['Ctpin7EM2'] = 8 * ['EM']
        allowedInputs['Ctpin7TAU1'] = 8 * ['HA']
        allowedInputs['Ctpin7TAU2'] = 8 * ['HA']

        allowedInputs['Ctpin8JET1'] = 10 * ['J']  # 3-bit JET
        allowedInputs['Ctpin8JET2'] = 15 * ['J']  # 2-bit JET
        allowedInputs['Ctpin8EN1'] = 8 * ['TE'] + 8 * ['XE'] + 8 * ['XS']
        allowedInputs['Ctpin8EN2'] = 8 * ['TE'] + 8 * ['XE']

        invalidThresholds = False
        for ithr, thr in enumerate(thresholds):
            try:
                allowedThr = allowedInputs[fullName][ithr]
                if thr[:len(allowedThr)] != allowedThr:
                    log.error(f"Threshold {thr} does not match expected type {allowedThr} at position {ithr} in connector {fullName}")
                    invalidThresholds = True
            except IndexError:
                log.error(f"Too many thresholds ({len(thresholds)}) provided for connector {fullName}, which only supports {len(allowedInputs[fullName])} thresholds")
                invalidThresholds = True
        if invalidThresholds:
            raise RuntimeError("Incorrect specification of legacy L1Calo thresholds")

    def checkCTPINconnectors(self):
        for conn in self.connectors:
            if conn.ctype == CType.CTPIN:
                if len(conn.triggerLines) > 31:
                    raise RuntimeError("Too many CTP inputs in %s: %i but a max of 31 are allowed" % (conn.name, len(conn.triggerLines)))

    def checkTOPObits(self):
        import re
        pattern = r"topo[1-3](e|$)"
        for conn in self.connectors:
            flatindexs = []
            if re.search(pattern, conn.name.lower()):
                for tl in conn.triggerLines:
                    for thr in conn.triggerLines[tl]:
                        for thrname in conn.triggerLines[tl][thr]:
                            if thrname.flatindex not in flatindexs:
                                flatindexs.append(thrname.flatindex)
                            else:
                                log.error("TOPO connector %s has duplicate flatindex %i", conn.name, thrname.flatindex)
                                raise RuntimeError("Duplicate flatindex found in TOPO algorithm")

    def checkCountCTPInputsOutput(self):
        from collections import namedtuple
        ctpInput = namedtuple('ctpInput', "name, conn, nbit")
        ctpInputs = []
        ctpUnusedInputs = []
        ctpOutputs = []
        thrNames = []
        ctpInputBitSets = dict()
        ctpInputNameSets = dict()
        ctpUnusedInputBitSets = dict()
        ctpUnusedInputNameSets = dict()
        for item in self.items:
            ctpOutputs.append(item.name)
            for thrName in item.thresholdNames():
                if thrName[:3] == 'ZB_':
                    thrName = thrName[3:]
                if thrName not in thrNames:
                    thrNames.append(thrName)
        thrNames_notFound = []
        for thrName in thrNames:
            thrName_found = False
            for conn in self.connectors:
                if conn.ctype != CType.ELEC:
                    for tl in conn.triggerLines:
                        if thrName == tl.name:
                            ctpInputs.append(ctpInput(name=thrName, conn=conn.name, nbit=tl.nbits))
                            thrName_found = True
                else:
                    for fpga in conn.triggerLines:
                        for clock in conn.triggerLines[fpga]:
                            for tl in conn.triggerLines[fpga][clock]:
                                if thrName == tl.name:
                                    ctpInputs.append(ctpInput(name=thrName, conn=conn.name, nbit=tl.nbits))
                                    thrName_found = True
            if not thrName_found:
                thrNames_notFound.append(thrName)

        for conn in self.connectors:
            if conn.ctype != CType.ELEC:
                for tl in conn.triggerLines:
                    thrName = tl.name
                    if thrName[:3] == 'ZB_':
                        thrName = thrName[3:]
                    usedInput = False
                    for ctpIn in ctpInputs:
                        if thrName == ctpIn.name:
                            usedInput = True
                    if not usedInput:
                        ctpUnusedInputs.append(ctpInput(name=thrName, conn=conn.name, nbit=tl.nbits))
            else:
                for fpga in conn.triggerLines:
                    for clock in conn.triggerLines[fpga]:
                        for tl in conn.triggerLines[fpga][clock]:
                            thrName = tl.name
                            if thrName[:3] == 'ZB_':
                                thrName = thrName[3:]
                            usedInput = False
                            for ctpIn in ctpInputs:
                                if thrName == ctpIn.name:
                                    usedInput = True
                            if not usedInput:
                                ctpUnusedInputs.append(ctpInput(name=thrName, conn=conn.name, nbit=tl.nbits))

        if len(thrNames_notFound) > 0:
            log.error("Thresholds [%s] are not found", ",".join(thrNames_notFound))
            log.error("Input thresholds are [%s]", ",".join([input.name for input in ctpInputs]))
            raise RuntimeError("Not all input thresholds found!")

        for ctpIn in ctpInputs:
            thrset = None
            thrName = ctpIn.name
            if thrName[:2] in ['EM', 'HA', 'XE', 'TE', 'XS']:
                thrset = 'legacyCalo'
            elif thrName[:1] == 'J':
                thrset = 'legacyCalo'
            elif thrName[:2] == 'MU':
                thrset = 'muon'
            elif thrName[:3] in ['MBT', 'AFP', 'BCM', 'CAL', 'NIM', 'ZDC', 'BPT', 'LUC', 'BMA']:
                thrset = 'detector'
            elif thrName[:6] == 'R2TOPO':
                thrset = 'legacyTopo'
            elif thrName[:1] in ['e', 'j', 'c', 'g']:
                thrset = 'topo1'
            elif thrName[:4] == 'TOPO':
                if 'Topo2' in ctpIn.conn:
                    thrset = 'topo2'
                elif 'Topo3' in ctpIn.conn:
                    thrset = 'topo3'

            if thrset not in ctpInputBitSets:
                ctpInputBitSets[thrset] = 0
                ctpInputNameSets[thrset] = []
            if thrName not in ctpInputNameSets[thrset]:
                ctpInputNameSets[thrset].append(thrName)
                ctpInputBitSets[thrset] += ctpIn.nbit

        for ctpIn in ctpUnusedInputs:
            thrset = None
            thrName = ctpIn.name
            if thrName[:2] in ['EM', 'HA', 'XE', 'TE', 'XS']:
                thrset = 'legacyCalo'
            elif thrName[:1] == 'J':
                thrset = 'legacyCalo'
            elif thrName[:2] == 'MU':
                thrset = 'muon'
            elif thrName[:3] in ['MBT', 'AFP', 'BCM', 'CAL', 'NIM', 'ZDC', 'BPT', 'LUC']:
                thrset = 'detector'
            elif thrName[:6] == 'R2TOPO':
                thrset = 'legacyTopo'
            elif thrName[:1] in ['e', 'j', 'c', 'g']:
                thrset = 'topo1'
            elif thrName[:4] == 'TOPO':
                if 'Topo2' in ctpIn.conn:
                    thrset = 'topo2'
                elif 'Topo3' in ctpIn.conn:
                    thrset = 'topo3'

            if thrset not in ctpUnusedInputBitSets:
                ctpUnusedInputBitSets[thrset] = 0
                ctpUnusedInputNameSets[thrset] = []
            if thrName not in ctpUnusedInputNameSets[thrset]:
                ctpUnusedInputNameSets[thrset].append(thrName)
                ctpUnusedInputBitSets[thrset] += ctpIn.nbit

        totalInputs = 0
        log.info("Check total number of CTP input and output bits:")
        log.info("Number of output bits: %i", len(ctpOutputs))
        for thrset in ctpInputBitSets:
            log.info("Used inputs in %s: %i thresholds and %i  bits", thrset, len(ctpInputNameSets[thrset]), ctpInputBitSets[thrset])
            if thrset is not None:
                log.debug("Threshold set %s: %s", thrset, ",".join(ctpInputNameSets[thrset]))
            else:
                log.warning("Unrecognised CTP input bits: %s", ",".join(ctpInputNameSets[thrset]))
            totalInputs += ctpInputBitSets[thrset]
        log.info("Number of used inputs bits: %i", totalInputs)
        totalUnusedInputs = 0
        for thrset in ctpUnusedInputBitSets:
            log.debug("Unused thresholds in %s: %i thresholds and %i  bits", thrset, len(ctpUnusedInputNameSets[thrset]), ctpUnusedInputBitSets[thrset])
            if thrset is not None:
                log.debug("Unused threshold set %s: %s", thrset, ",".join(ctpUnusedInputNameSets[thrset]))
            else:
                log.debug("Unrecognised CTP input bits: %s", ",".join(ctpUnusedInputNameSets[thrset]))
            totalUnusedInputs += ctpUnusedInputBitSets[thrset]
        log.debug("Number of un-used inputs bits: %i", totalUnusedInputs)

        # Fail menu generation for menus going to P1:
        if (totalInputs > Limits.MaxTrigItems or len(ctpOutputs) > Limits.MaxTrigItems):
            if 'AllCTPIn' in self.menuInfo.menuBaseName:
                log.warning(f"Input or output bits limit of {Limits.MaxTrigItems} exceeded in the dummy CTP menu -- OK")
            else:
                raise RuntimeError("Both the numbers of inputs and outputs need to be not greater than %i in a physics menu!" % Limits.MaxTrigItems)

    # Avoid that L1 item is defined only for BGRP0 as this include also the CALREQ BGRP2 (ATR-24781)
    def checkBGRP(self):
        for item in self.items:
            if len(item.bunchGroups) == 1 and item.bunchGroups[0] == 'BGRP0':
                raise RuntimeError("L1 item %s is defined with only BGRP0, ie it can trigger also in the CALREQ BGRP2 bunches. Please add another bunch group (ATR-24781)" % item.name)
            if 'BGRP2' in item.bunchGroups:
                if isinstance(item.logic, ThrCondition):
                    thrtype = item.logic.threshold().ttype
                    if thrtype in ThrType.CaloTypes():
                        # The LAr Digital Trigger sends an "align frame" to the FEXes in BCID 3500 (in BGRP2)
                        # No trigger can be sent during this align frame, so we block all calo triggers from this BGRP
                        raise RuntimeError(f"L1 item {item.name} with threshold type {thrtype} is in CALREQ BGRP2. This is not allowed!")

    def checkItemsHaveInputs(self):
        all_ctpin = []
        connected_boards = []
        for layout, connectors in self.ctp.inputConnectors.items():
            for _, contents in connectors.items():
                if layout == 'ctpin':
                    # Ignore empty
                    connected_boards += [board for board in contents.values() if board]
                else:
                    connected_boards.append(contents)

        for conn_name in connected_boards:
            conn = self.connectors[conn_name]
            if conn.ctype != CType.ELEC:
                for tl in conn.triggerLines:
                    thrName = tl.name
                    if thrName[:3] == 'ZB_':
                        thrName = thrName[3:]
                    all_ctpin.append(thrName)
            else:
                for fpga in conn.triggerLines:
                    for clock in conn.triggerLines[fpga]:
                        for tl in conn.triggerLines[fpga][clock]:
                            thrName = tl.name
                            if thrName[:3] == 'ZB_':
                                thrName = thrName[3:]
                            all_ctpin.append(thrName)

        for item in self.items:
            for thrName in item.thresholdNames():
                if thrName[:3] == 'ZB_':
                    thrName = thrName[3:]
                if thrName not in all_ctpin:
                    raise RuntimeError(
                        f'checkItemsHaveInputs: Threshold {thrName} used by {item.name} is not on a board connected to CTP')

    def checkHFmonitoring(self):
        requiredItems = [
            "L1_BCM_2A_FIRSTINTRAIN",
            "L1_BCM_2C_FIRSTINTRAIN"
        ]
        missingItems = [item for item in requiredItems if self.items.items[item].monitorsHF == 0]
        if missingItems:
            raise RuntimeError(
                f'The HF monitoring flags for {", ".join(missingItems)} are missing')
