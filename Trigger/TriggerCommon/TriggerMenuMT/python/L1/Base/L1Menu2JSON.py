# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from .L1Menu import L1Menu as L1MenuRun3
from .L1MenuRun4 import L1MenuRun4
class L1MenuJSONConverter:

    def __init__(self, l1menu: L1MenuRun3 | L1MenuRun4, outputFile = None, bgsOutputFile = None, inputFile = None ):
        self.menu: L1MenuRun3 | L1MenuRun4 = l1menu
        self.inputFile     = inputFile
        self.outputFile    = outputFile
        self.bgsOutputFile = bgsOutputFile

    def writeJSON(self,pretty=False):
        import json

        if not self.outputFile:
            log.warning("Can't write json file since no name was provided")
            return

        # L1Menu json
        confObj = self.generateJSON()
        with open( self.outputFile, mode="wt" ) as fh:
            json.dump(confObj, fh, indent = 4 if pretty else None, separators=(',', ': '))
            fh.write("\n")
        log.info("Wrote %s", self.outputFile)

        if self.bgsOutputFile is not None:
            confObj = self.generateJsonBunchgroupset()
            with open( self.bgsOutputFile, mode="wt" ) as fh:
                json.dump(confObj, fh, indent = 4 if pretty else None, separators=(',', ': '))
                fh.write("\n")
            log.info("Wrote %s", self.bgsOutputFile)

        return self.outputFile


    def generateJSON(self):
        if isinstance(self.menu, L1MenuRun4):
            return self.generateJSONRun4(self.menu)
        else:
            return self.generateJSONRun3(self.menu)

    def generateJSONRun4(self, menu: L1MenuRun4):
        confObj = {
            "filetype": "l1menu",
            "name": menu.menuName,
            "run": 4,
            "items": menu.items.json(),
            "thresholds": {}
        }

        # thresholds
        confObj["thresholds"]["internal"] = {
            "type": "internal",
            "names": [ f"BGRP{bg.internalNumber}" for bg in menu.ctp.bunchGroupSet.bunchGroups] + \
                     [ f"RNDM{i}" for i in range(0,len(menu.ctp.random.names)) ],
            "randoms": { f"RNDM{i}": { "cut" : c } for i,c in enumerate(menu.ctp.random.cuts ) }
        }
        confObj["thresholds"].update(menu.thresholds.json() )

        confObj["global"] = menu.globalAlgos.json()

        # board definition
        confObj["boards"] = menu.boards.json()

        # connectors definition
        confObj["connectors"] = menu.connectors.json()

        # CTP input cabling definition
        # confObj["ctp"] = menu.ctp.json()

        return confObj

    def generateJSONRun3(self, menu: L1MenuRun3):
        confObj = {
            "filetype": "l1menu",
            "name": menu.menuName,
            "run": 3,  # will be useful for later (we also record this for Run 1 and 2)
            "items": menu.items.json(),
            "thresholds": {}
        }

        # thresholds
        confObj["thresholds"]["internal"] = {
            "type": "internal",
            "names": [ f"BGRP{bg.internalNumber}" for bg in menu.ctp.bunchGroupSet.bunchGroups] + \
                     [ f"RNDM{i}" for i in range(0,len(menu.ctp.random.names)) ],
            "randoms": { f"RNDM{i}": { "cut" : c } for i,c in enumerate( menu.ctp.random.cuts ) }
        }

        # run 3 thresholds
        confObj["thresholds"].update( menu.thresholds.json() )

        # legacy calo thresholds
        confObj["thresholds"]["legacyCalo"] = menu.thresholds.jsonLegacy()

        # topo algorithms
        confObj["topoAlgorithms"] = menu.topoAlgos.json()

        # board definition
        confObj["boards"] = menu.boards.json()

        # connectors definition
        confObj["connectors"] = menu.connectors.json()

        # CTP input cabling definition
        confObj["ctp"] = menu.ctp.json()

        return confObj

    def generateJsonBunchgroupset(self):
        confObj = {
            "filetype": "bunchgroupset",
            "name": self.menu.menuName,
            "bunchGroups": self.menu.ctp.bunchGroupSet.json()
        }
        return confObj
