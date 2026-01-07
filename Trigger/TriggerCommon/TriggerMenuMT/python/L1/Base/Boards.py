# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__all__ = ['MenuBoardsCollection', 'BoardType']

from enum import Enum

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class BoardType(Enum):
    NONE = 1
    MUCTPI = 2
    TOPO = 3
    CTPIN = 4
    MERGER = 5
    L0GLOBAL = 6
    def __repr__(self):
        return self.name
    def __str__(self):
        return self.name
    @staticmethod
    def fromBoardName(name):
        if 'muctpi' in name.lower():
            btype = BoardType.MUCTPI
        elif 'merger' in name.lower():
            btype = BoardType.MERGER
        elif 'topo' in name.lower():
            btype = BoardType.TOPO
        elif 'ctpin' in name.lower():
            btype = BoardType.CTPIN
        elif 'ctpin' in name.lower():
            btype = BoardType.CTPIN
        elif 'l0global' in name.lower():
            btype = BoardType.L0GLOBAL
        else:
            raise RuntimeError("No BoardType defined for board %s" % name)
        return btype


class MenuBoardsCollection(object):
    def __init__(self):
        self.boards = {}

    def addBoard(self, boardDef):
        name = boardDef["name"]
        btype = BoardType.fromBoardName(name)
        isLegacy = 'legacy' in boardDef
        self.boards[name] = Board(name, btype, isLegacy)
        if "connectors" in boardDef:
            self.boards[name].addOutputConnectorNames([c["name"] for c in boardDef["connectors"]])
        return self.boards[name]

    def json(self):
        confObj = {boardName : self.boards[boardName].json() for boardName in sorted(self.boards)}
        return confObj


class Board(object):
    def __init__(self, name, btype, isLegacy = False):
        self.name = name
        self.btype = btype
        self.isLegacy = isLegacy
        self.outputConnectors = []
        self.inputConnectors = []

    def addOutputConnectorNames(self, connName ):
        self.outputConnectors += connName

    def json(self):
        confObj = {}
        confObj["type"] = str(self.btype)
        if self.isLegacy:
            confObj["legacy"] = self.isLegacy
        confObj["connectors"] = self.outputConnectors
        # inputConnectors only exist for merger boards
        if confObj["type"] == BoardType.MERGER:
            confObj["inputConnectors"] = self.inputConnectors
        return confObj
