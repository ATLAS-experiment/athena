# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
import AthenaConfiguration.AtlasSemantics # noqa: F401 (load ATLAS-specific semantics)
from AthenaCommon.Configurable import Configurable
from AthenaCommon.ConfigurableDb import getConfigurable
from GaudiConfig2 import Configurables as _cfgs


def isComponentAccumulatorCfg():
    """Returns true if the python fragment is ComponentAccumulator-based"""

    if ("AthenaCommon.Include" not in sys.modules
        or not Configurable._useGlobalInstances):
        return True
    else:
        return False


class _compFactory1():
    """Factory providing legacy Configurables with global namespace"""

    def __getattr__(self,cfgName):
        if not cfgName.startswith("__"):
            return getConfigurable(cfgName.replace("::","__"),assumeCxxClass=False)

    def getComp(self, cfgName):
        return getConfigurable(cfgName.replace("::","__"),assumeCxxClass=False)

    def getComps(self, *manyNames):
        return [getConfigurable(cfgName.replace("::","__"),assumeCxxClass=False) for cfgName in manyNames]


class _compFactory2():
    """Factory providing GaudiConfig2 Configurable without global namespace"""

    def __getattr__(self,cfgName):
        if not cfgName.startswith("__"):
            return getattr(_cfgs,cfgName)

    def getComp(self, oneName):
        return _cfgs.getByType(oneName)

    def getComps(self, *manyNames):
        return [_cfgs.getByType(cfgName) for cfgName in manyNames]


class _compFactory():
    """Return Configurable factory for legacy/CA jobs"""

    def _getFactory(self):
        return _compFactory2() if isComponentAccumulatorCfg() else _compFactory1()

    def __getattr__(self,cfgName):
        return getattr(self._getFactory(),cfgName)

    def getComp(self, oneName):
        return self._getFactory().getComp(oneName)

    def getComps(self, *manyNames):
        return self._getFactory().getComps(*manyNames)


CompFactory=_compFactory()
