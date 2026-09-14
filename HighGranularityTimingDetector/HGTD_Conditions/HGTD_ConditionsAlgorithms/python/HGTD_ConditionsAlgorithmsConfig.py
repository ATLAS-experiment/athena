"""Define functions to configure HGTD conditions algorithms

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HGTD_AlignCondAlgCfg(flags, name="HGTD_AlignCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured HGTD_AlignCondAlg for HGTD"""
    acc = ComponentAccumulator()
    kwargs.setdefault("DetManagerName", "HGTD")
    kwargs.setdefault("ReadKey", flags.HGTD.Geometry.alignmentFolder)
    kwargs.setdefault("WriteKey", "HGTDAlignmentStore")
    acc.addCondAlgo(CompFactory.HGTD_AlignCondAlg(name, **kwargs))
    return acc

def HGTD_DetectorElementCondAlgCfg(flags, name="HGTD_DetectorElementCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured HGTD_DetectorElementCondAlg for HGTD"""
    acc = ComponentAccumulator()
    acc.merge(HGTD_AlignCondAlgCfg(flags))
    kwargs.setdefault("DetManagerName", "HGTD")
    kwargs.setdefault("ReadKey", "HGTDAlignmentStore")
    kwargs.setdefault("WriteKey", "HGTD_DetectorElementCollection")
    acc.addCondAlgo(CompFactory.HGTD_DetectorElementCondAlg(name, **kwargs))
    return acc
