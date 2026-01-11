# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from TriggerMenuMT.L1.Menu.MenuMapping import MenuMetaInfo
log = logging.getLogger(__name__)

from ..Config.MonitorDef import MonitorDef
from ..Config.CTPInputConfig import CTPInputConfig
from .BunchGroupSet import BunchGroupSet
from .MonCounters import MenuMonCountersCollection

class CTP:

    def __init__(self, run):
        assert(run in [3,4]), "CTP configuration only defined for Run3 and Run4"
        if run == 3:
            self.inputConnectors = CTPInputConfig.cablingLayout()
        else:
            self.inputConnectors = CTPInputConfig.cablingLayoutRun4()
        self.random          = Random( names = ['Random0', 'Random1', 'Random2', 'Random3'], cuts = [1, 1, 1, 1] )
        self.bunchGroupSet   = BunchGroupSet()
        self.counters        = MenuMonCountersCollection()   # monitoring counters in the menu

        # Customisations via flags

    def setBunchGroupSetName(self, name):
        self.bunchGroupSet.name = name
        return self.bunchGroupSet

    def setupMonitoring(self, menuName, menuItems, menuThresholds, connectors, menuFullName, *, menuInfo: MenuMetaInfo | None = None):
        ##  # add the CTPIN counters
        ##  for counter in MonitorDef.ctpinCounters( menuThresholds ):
        ##      self.counters.addCounter( counter )

        # add the CTPMon counters (selection defined in L1/Config/MonitorDef.py)
        for counter in MonitorDef.ctpmonCounters( menuThresholds, connectors ):
            self.counters.addCounter( counter )

        # add the CTPIN counters (selection defined in L1/Config/MonitorDef.py)
        for counter in MonitorDef.ctpinCounters( menuThresholds, connectors, self.inputConnectors["ctpin"] ):
            self.counters.addCounter( counter )

        # mark the L1 Items that they should be monitored
        MonitorDef.applyItemCounter( menuName, menuItems, menuFullName, menuInfo=menuInfo )

    def checkConnectorAvailability(self, availableConnectors, menuInputFile: str):
        inputConnectorList = []
        inputConnectorList += self.inputConnectors.get("optical", {}).values()
        inputConnectorList += self.inputConnectors.get("electrical", {}).values()
        inputConnectorList += self.inputConnectors.get("ctpin", {}).get("slot7", {}).values()
        inputConnectorList += self.inputConnectors.get("ctpin", {}).get("slot8", {}).values()
        inputConnectorList += self.inputConnectors.get("ctpin", {}).get("slot9", {}).values()
        for connName in inputConnectorList:
            if connName != '' and connName not in availableConnectors:
                msg = (
                    f"Connector '{connName}' requested in L1/Config/CTPInputConfig.py not defined as menu input. "
                    f"Please add it to L1/Menu/Menu_{menuInputFile}.py"
                )
                log.error(msg)
                raise RuntimeError(msg)

    def json(self):
        confObj = {
            "inputs": self.inputConnectors,
            "monitoring": self.counters.json()
        }
        return confObj



class Random(object):
    def __init__(self, names, cuts):
        self.names = names
        self.cuts  = cuts

