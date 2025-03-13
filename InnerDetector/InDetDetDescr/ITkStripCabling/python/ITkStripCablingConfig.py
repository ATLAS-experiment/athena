# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkStripCablingCondAlgCfg(flags, name="ITkStripCablingCondAlg"):
    cfg = ComponentAccumulator()
    from PathResolver import PathResolver
    ITkStripCablingCondAlg = CompFactory.ITkStripCablingAlg(name,DataSource=PathResolver.FindCalibFile("ITkStripCabling/ITkStripCabling.dat"))
    cfg.addCondAlgo(ITkStripCablingCondAlg)
    return cfg

def ITkStripCablingToolCfg(flags, name="ITkStripCablingTool"):
    cfg = ComponentAccumulator()

    # For SCT_ID used in SCT_CablingTool
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    cfg.merge(GeoModelCfg(flags))

    cfg.merge(ITkStripCablingCondAlgCfg(flags))

    cfg.setPrivateTools(CompFactory.ITkStripCablingTool(name))
    return cfg


