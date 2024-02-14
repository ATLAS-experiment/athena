#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory

def ITkPixelRawDataProviderAlgCfg(flags, RDOKey="PixelRDOs", **kwargs):
    """ Main function to configure ITkPixel raw data decoding """

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
    acc = ITkPixelReadoutManagerCfg(flags)

    from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
    regSelTool = acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags))

    suffix = kwargs.pop("suffix","")
    decoder = CompFactory.ITkPixelRodDecoder(name="ITkPixelRodDecoder"+suffix)
    
    providerTool =  CompFactory.ITkPixelRawDataProviderTool(name="ITkPixelRawDataProviderTool"+suffix,
                                                         Decoder = decoder)

    acc.addEventAlgo(CompFactory.ITkPixelRawDataProvider(RDOKey = RDOKey,
                                                      RegSelTool = regSelTool, 
                                                      ProviderTool = providerTool,
                                                      **kwargs))
    return acc


def TrigITkPixelRawDataProviderAlgCfg(flags, suffix, RoIs):
    trigargs = {
        'name' : 'TrigPixelRawDataProvider'+suffix,
        'suffix' : suffix,
        'RoIs' : RoIs,   
        'isRoI_Seeded': True,
        'RDOCacheKey' : 'PixRDOCache',
        'BSErrorsCacheKey' : 'PixBSErrCache'
    }
    return ITkPixelRawDataProviderAlgCfg(flags, **trigargs)