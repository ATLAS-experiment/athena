# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkStripCablingCondAlgCfg(flags, name="ITkStripCablingCondAlg",**kwargs):
    cfg = ComponentAccumulator()
    kwargs.setdefault("DataSource","ITkStripCabling/ITkStripCabling.dat")
    cfg.addCondAlgo(CompFactory.ITkStripCablingAlg(name,**kwargs))
    return cfg

def ITkStripCablingToolCfg(flags, name="ITkStripCablingTool"):
    cfg = ComponentAccumulator()

    # For SCT_ID used in SCT_CablingTool
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    cfg.merge(GeoModelCfg(flags))

    cfg.merge(ITkStripCablingCondAlgCfg(flags))

    cfg.setPrivateTools(CompFactory.ITkStripCablingTool(name))
    return cfg


