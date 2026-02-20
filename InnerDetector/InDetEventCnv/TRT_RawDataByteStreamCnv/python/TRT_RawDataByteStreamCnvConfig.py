# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def TRT_CablingSvcCfg(flags):
    """Return a ComponentAccumulator for TRT_CablingSvc service"""
    acc = ComponentAccumulator()
    # Properly configure MC/data for TRT cabling
    acc.addPublicTool(CompFactory.TRT_FillCablingData_DC3(RealData=not flags.Input.isMC))
    # Setup TRT cabling service
    acc.addService(CompFactory.TRT_CablingSvc())
    return acc


def TRT_RodDecoderCfg(flags, name="TRT_RodDecoder", **kwargs):
    """Return a ComponentAccumulator for TRT ROD decoder"""
    acc = ComponentAccumulator()
    kwargs.setdefault("SortCollections", flags.Common.ProductionStep is ProductionStep.MinbiasPreprocessing)
    acc.setPrivateTools(CompFactory.TRT_RodDecoder(name, **kwargs))
    return acc


def TRTRawDataProviderToolCfg(flags, name="InDetTRTRawDataProviderTool", **kwargs):
    """Return a ComponentAccumulator for TRT raw data provider tool"""
    acc = ComponentAccumulator()
    
    kwargs.setdefault("LVL1IDKey", "TRT_LVL1ID")
    kwargs.setdefault("BCIDKey", "TRT_BCID")

    kwargs.setdefault("Decoder", acc.popToolsAndMerge(TRT_RodDecoderCfg(flags)))

    acc.setPrivateTools(CompFactory.TRTRawDataProviderTool(name, **kwargs))
    return acc


def TRTRawDataProviderCfg(flags, name="TRTRawDataProvider", **kwargs):
    """Return a ComponentAccumulator for TRT raw data provider"""
    acc = TRT_CablingSvcCfg(flags)

    if not flags.Input.isMC:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, "/TRT/Onl/ROD/Compress", "TRT_ONL", className="CondAttrListCollection"))

    if 'ProviderTool' not in kwargs:
        kwargs.setdefault("ProviderTool", acc.popToolsAndMerge(TRTRawDataProviderToolCfg(flags)))

    from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
    kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(regSelTool_TRT_Cfg(flags)))

    if flags.Common.ProductionStep is ProductionStep.MinbiasPreprocessing:
        kwargs.setdefault("RDOKey", f"{flags.Overlay.BkgPrefix}TRT_RDOs")

    acc.addEventAlgo(CompFactory.TRTRawDataProvider(name, **kwargs))
    return acc


def TrigTRTRawDataProviderCfg(flags : AthConfigFlags, RoIs : str, **kwargs):
    acc = ComponentAccumulator()

    suffix = flags.Tracking.ActiveConfig.input_name
    providerToolName = f"TrigTRTRawDataProviderTool_{suffix}"
    providerName = f"TrigTRTRawDataProvider_{suffix}"

    providerTool = acc.popToolsAndMerge(
        TRTRawDataProviderToolCfg(flags,
                                  name=providerToolName,
                                  StoreInDetTimeCollections=False)
    )
    
    kwargs.setdefault("ProviderTool", providerTool)
    kwargs.setdefault('isRoI_Seeded', True)
    kwargs.setdefault('RoIs',         RoIs)
    kwargs.setdefault('RDOKey',       'TRT_RDOs_TRIG')
    kwargs.setdefault('RDOCacheKey',  flags.Trigger.InDetTracking.TRTRDOCacheKey)
    
    acc.merge(TRTRawDataProviderCfg(flags, name = providerName, **kwargs))
    return acc
