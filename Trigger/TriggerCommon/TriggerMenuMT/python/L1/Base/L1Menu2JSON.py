# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from .L1Menu import L1Menu
class L1MenuJSONConverter(object):

    def __init__(self, l1menu: L1Menu, outputFile = None, bgsOutputFile = None, inputFile = None ):
        self.menu          = l1menu
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
        if self.menu.isRun4Menu:
            return self.generateJSONRun4()
        else:
            return self.generateJSONRun3()

    def generateJSONRun4(self):
        confObj = {
            "filetype": "l1menu",
            "name": self.menu.menuName,
            "run": 4,
            "items": self.menu.items.json(),
            "thresholds": {}
        }

        # thresholds
        confObj["thresholds"]["internal"] = {
            "type": "internal",
            "names": [ f"BGRP{bg.internalNumber}" for bg in self.menu.ctp.bunchGroupSet.bunchGroups] + \
                     [ f"RNDM{i}" for i in range(0,len(self.menu.ctp.random.names)) ],
            "randoms": { f"RNDM{i}": { "cut" : c } for i,c in enumerate( self.menu.ctp.random.cuts ) }
        }
        confObj["thresholds"].update( self.menu.thresholds.json() )

        confObj["L0Global"] = self.menu.topoAlgos.json()

        # board definition
        confObj["boards"] = self.menu.boards.json()

        # connectors definition
        confObj["connectors"] = self.menu.connectors.json()

        # CTP input cabling definition
        # confObj["ctp"] = self.menu.ctp.json()

        return confObj

    def generateJSONRun3(self):
        confObj = {
            "filetype": "l1menu",
            "name": self.menu.menuName,
            "run": 3,  # will be useful for later (we also record this for Run 1 and 2)
            "items": self.menu.items.json(),
            "thresholds": {}
        }

        # thresholds
        confObj["thresholds"]["internal"] = {
            "type": "internal",
            "names": [ f"BGRP{bg.internalNumber}" for bg in self.menu.ctp.bunchGroupSet.bunchGroups] + \
                     [ f"RNDM{i}" for i in range(0,len(self.menu.ctp.random.names)) ],
            "randoms": { f"RNDM{i}": { "cut" : c } for i,c in enumerate( self.menu.ctp.random.cuts ) }
        }

        # run 3 thresholds
        confObj["thresholds"].update( self.menu.thresholds.json() )

        # legacy calo thresholds
        confObj["thresholds"]["legacyCalo"] = self.menu.thresholds.jsonLegacy()

        # topo algorithms
        confObj["topoAlgorithms"] = self.menu.topoAlgos.json()

        # board definition
        confObj["boards"] = self.menu.boards.json()

        # connectors definition
        confObj["connectors"] = self.menu.connectors.json()

        # CTP input cabling definition
        confObj["ctp"] = self.menu.ctp.json()

        return confObj


    def generateJsonBunchgroupset(self):
        confObj = {
            "filetype": "bunchgroupset",
            "name": self.menu.menuName,
            "bunchGroups": self.menu.ctp.bunchGroupSet.json()
        }
        return confObj
