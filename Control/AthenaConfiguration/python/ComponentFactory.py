# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
import AthenaConfiguration.AtlasSemantics # noqa: F401 (load ATLAS-specific semantics)
from AthenaCommon.Configurable import Configurable
from GaudiConfig2 import Configurables as cfg2


def isComponentAccumulatorCfg():
    """Returns true if the python fragment is ComponentAccumulator-based"""

    if ("AthenaCommon.Include" not in sys.modules
        or not Configurable._useGlobalInstances):
        return True
    else:
        return False


def _getConf1(name):
    """Return legacy Configurable class with given name"""
    from AthenaCommon.ConfigurableDb import getConfigurable
    return getConfigurable(name.replace("::","__"), assumeCxxClass=False)


class _compFactory():
    """Return Configurable factory for legacy/CA jobs"""

    def __getattr__(self, name):
        """Return Configurable class with given name"""
        if not name.startswith("__"):
            return getattr(cfg2, name) if isComponentAccumulatorCfg() else _getConf1(name)

    def getComp(self, name):
        """Return Configurable class with given name"""
        return cfg2.getByType(name) if isComponentAccumulatorCfg() else _getConf1(name)

    def getComps(self, *names):
        """Return list of Configurable classes with given names"""
        return [self.getComp(name) for name in names]


CompFactory = _compFactory()
