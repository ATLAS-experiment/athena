# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from .GlobalAlgorithm import GlobalAlgorithm
    
class GlobalAlgorithmsCollection:
    """Wrapper class for encapsulating the list of all global algorithms."""
    def __init__(self):
        # all algos that are in menu (new and legacy)
        self.globalAlgos: dict[str, GlobalAlgorithm] = {}

    def addAlgo(self, algo: GlobalAlgorithm):
        if algo.name in self.globalAlgos:
            raise RuntimeError("Trying to add topo algorithm %s a second time" % algo.name)
        self.globalAlgos[algo.name] = algo

    def json(self):
        confObj = {}
        for alg in self.globalAlgos.values():
            algType = alg.alg_type
            if algType.key not in confObj:
                confObj[algType.key] = {}
            confObj[algType.key][alg.name] = alg.json()
        return confObj
