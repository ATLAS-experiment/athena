# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
  ComponentAccumulator tool configuration for ISF_ActsTools
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsFatrasSimToolCfg(flags, name="ISF_ActsFatrasSimTool", **kwargs):
    """Return ISF_FatrasSimHitCreatorID configured with ComponentAccumulator"""
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault('TrackingGeometryTool', acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))

    kwargs.setdefault("MaxSteps", 2000)

    from ISF_Services.ISF_ServicesConfig import TruthServiceCfg
    kwargs.setdefault("TruthRecordService", acc.getPrimaryAndMerge(TruthServiceCfg(flags)).name)
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RNGService", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.ISF.ActsFatrasSimTool(name, **kwargs))
    return acc
