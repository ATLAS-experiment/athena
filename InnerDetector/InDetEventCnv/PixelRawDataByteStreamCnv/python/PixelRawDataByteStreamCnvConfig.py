#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg, PixelHitDiscCnfgAlgCfg


def PixelRawDataProviderToolCfg(flags, prefix="", suffix="", storeInDetTimeCollections=True):
    acc = ComponentAccumulator()
    decoder = CompFactory.PixelRodDecoder(CheckDuplicatedPixel = False if "data15" in flags.Input.ProjectName else True)
    acc.setPrivateTools(CompFactory.PixelRawDataProviderTool(Decoder = decoder, StoreInDetTimeCollections = storeInDetTimeCollections))
    return acc


def PixelRawDataProviderAlgCfg(flags, **kwargs):
    """ Main function to configure Pixel raw data decoding """
    acc = PixelCablingCondAlgCfg(flags)
    acc.merge(PixelHitDiscCnfgAlgCfg(flags))

    if flags.Common.ProductionStep is ProductionStep.MinbiasPreprocessing:
        kwargs.setdefault("RDOKey", f"{flags.Overlay.BkgPrefix}PixelRDOs")
    else:
        kwargs.setdefault("RDOKey", "PixelRDOs")

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    acc.merge (PixelReadoutManagerCfg(flags))

    from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
    regSelTool = acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags))
    storeInDetTimeCollections = kwargs.pop("StoreInDetTimeCollections",True)

    prefix = kwargs.pop("prefix","")
    suffix = kwargs.pop("suffix","")
    providerTool = acc.popToolsAndMerge(PixelRawDataProviderToolCfg(flags, prefix, suffix, storeInDetTimeCollections))
    acc.addEventAlgo(CompFactory.PixelRawDataProvider(RegSelTool = regSelTool,
                                                      ProviderTool = providerTool,
                                                      **kwargs))
    return acc


def TrigPixelRawDataProviderAlgCfg(flags, suffix, RoIs, **kwargs):
    kwargs.setdefault('name', 'TrigPixelRawDataProvider'+suffix)
    kwargs.setdefault('prefix', "Trig")
    kwargs.setdefault('suffix', suffix)
    kwargs.setdefault('RoIs', RoIs)
    kwargs.setdefault('isRoI_Seeded', True)
    kwargs.setdefault('RDOCacheKey',      flags.Trigger.InDetTracking.PixRDOCacheKey)
    kwargs.setdefault('BSErrorsCacheKey', flags.Trigger.InDetTracking.PixBSErrCacheKey)
    kwargs.setdefault('StoreInDetTimeCollections', False)
    return PixelRawDataProviderAlgCfg(flags, **kwargs)
