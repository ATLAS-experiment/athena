# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def TRT_CablingSvcCfg(flags):
    """Return a ComponentAccumulator for TRT_CablingSvc service"""
    acc = ComponentAccumulator()
    # Properly configure MC/data for TRT cabling
    tool = CompFactory.TRT_FillCablingData_DC3(RealData=not flags.Input.isMC)
    acc.addPublicTool(tool)
    # Setup TRT cabling service
    acc.addService(CompFactory.TRT_CablingSvc())
    return acc


def TRTRawDataProviderCfg(flags, name="TRTRawDataProvider", **kwargs):
    """Return a ComponentAccumulator for TRT raw data provider"""
    acc = TRT_CablingSvcCfg(flags)

    if not flags.Input.isMC:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, "/TRT/Onl/ROD/Compress", "TRT_ONL", className="CondAttrListCollection"))

    if 'ProviderTool' not in kwargs:
        kwargs.setdefault("ProviderTool", CompFactory.TRTRawDataProviderTool("InDetTRTRawDataProviderTool",
                                                                             LVL1IDKey = "TRT_LVL1ID",
                                                                             BCIDKey = "TRT_BCID"))

    from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
    kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(regSelTool_TRT_Cfg(flags)))

    providerAlg = CompFactory.TRTRawDataProvider(name, **kwargs)
    acc.addEventAlgo(providerAlg)
    return acc

def TrigTRTRawDataProviderCfg(flags : AthConfigFlags, RoIs : str, **kwargs):

    suffix = flags.Tracking.ActiveConfig.input_name
    providerToolName = f"TrigTRTRawDataProviderTool_{suffix}"
    providerName = f"TrigTRTRawDataProvider_{suffix}"

    providerTool = CompFactory.TRTRawDataProviderTool(name = providerToolName,
                                                      StoreInDetTimeCollections = False)
    
    kwargs.setdefault("ProviderTool", providerTool)
    kwargs.setdefault('isRoI_Seeded', True)
    kwargs.setdefault('RoIs',         RoIs)
    kwargs.setdefault('RDOKey',       'TRT_RDOs_TRIG')
    kwargs.setdefault('RDOCacheKey',  'TrtRDOCache')
    
    return TRTRawDataProviderCfg(flags, name = providerName, **kwargs)

def TRTOverlayRawDataProviderAlgCfg(flags, name="TRTRawDataProvider", **kwargs):
    """Return a ComponentAccumulator for TRT raw data provider for data overlay."""
    kwargs.setdefault("RDOKey", flags.Overlay.BkgPrefix + "TRT_RDOs")
    return TRTRawDataProviderCfg(flags, name, **kwargs)
