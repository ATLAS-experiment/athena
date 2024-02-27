#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from PixelConditionsAlgorithms.PixelConditionsConfig import PixelCablingCondAlgCfg, PixelHitDiscCnfgAlgCfg

def PixelRawDataProviderAlgCfg(flags, RDOKey="PixelRDOs", **kwargs):
    """ Main function to configure Pixel raw data decoding """
    acc = PixelCablingCondAlgCfg(flags)
    acc.merge(PixelHitDiscCnfgAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    acc.merge (PixelReadoutManagerCfg(flags))

    from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
    regSelTool = acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags))

    suffix = kwargs.pop("suffix","")
    if 'Decoder' not in kwargs:
        decoder = CompFactory.PixelRodDecoder(name="PixelRodDecoder"+suffix,
                                              CheckDuplicatedPixel = False if "data15" in flags.Input.ProjectName else True
                                              )

    if 'ProviderTool' not in kwargs:
        kwargs.setdefault("ProviderTool", CompFactory.PixelRawDataProviderTool(name="PixelRawDataProviderTool"+suffix,
                                                                               Decoder = decoder))

    acc.addEventAlgo(CompFactory.PixelRawDataProvider(RDOKey = RDOKey,
                                                      RegSelTool = regSelTool, 
                                                      **kwargs))
    return acc


def TrigPixelRawDataProviderAlgCfg(flags, suffix, RoIs, **kwargs):
    decoder = CompFactory.PixelRodDecoder(name="TrigPixelRodDecoder"+suffix,
                                          CheckDuplicatedPixel = False if "data15" in flags.Input.ProjectName else True
                                          )
    providerTool =  CompFactory.PixelRawDataProviderTool(name="TrigPixelRawDataProviderTool"+suffix,
                                                         Decoder = decoder,
                                                         StoreInDetTimeCollections = False)
    kwargs.setdefault('name', 'TrigPixelRawDataProvider'+suffix)
    kwargs.setdefault('suffix', suffix)
    kwargs.setdefault('RoIs', RoIs)
    kwargs.setdefault('isRoI_Seeded', True)
    kwargs.setdefault('RDOCacheKey', 'PixRDOCache')
    kwargs.setdefault('BSErrorsCacheKey', 'PixBSErrCache')
    kwargs.setdefault("ProviderTool", providerTool)
    return PixelRawDataProviderAlgCfg(flags, **kwargs)
