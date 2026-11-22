#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def addITkPixelCabling(flags):
    from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
    acc = ComponentAccumulator()
    
    if flags.ITk.Conditions.PixelTestCablingFallback:
        acc.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))
    else:
        acc.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=False))
    return acc


def ITkPixelHitSortingToolCfg(flags, name = "ITkPixelHitSortingTool", **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
    acc.merge(ITkPixelReadoutManagerCfg(flags))

    acc.setPrivateTools(CompFactory.ITkPixelHitSortingTool(name, **kwargs))
    return acc

def ITkPixelEncodingToolCfg(flags, name = "ITkPixelEncodingTool", **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ITkPixelEncodingTool(name, **kwargs))
    return acc

def ITkPixelTranslatorAlgCfg(flags, name = "ITkPixelTranslatorAlg", **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    acc.addEventAlgo(CompFactory.ITkPixelTranslatorAlg(name, **kwargs))

    return acc

def ITkPixelDecodingToolCfg(flags, name = "ITkPixelDecodingTool",
        ExpectedDataSize = 2000000,
        FailOnDecodingError = False,
        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    acc.merge(addITkPixelCabling(flags))

    acc.setPrivateTools(CompFactory.ITkPixelDecodingTool(name,
        ExpectedDataSize = ExpectedDataSize,
        FailOnDecodingError = FailOnDecodingError,
        **kwargs))

    return acc

def ITkPixelDecodingAlgCfg(flags, name = "ITkPixelDecodingAlg",
        pixelRDOKey = "ITkPixelRDOs",
        FailOnDecodingError = False,
        ExpectedDataSize = 2000000,
        RegSelTool = None,
        RoIs = None,
        isRoI_Seeded = False,
        RDOCacheKey = "",
        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    tool_args = kwargs
    if isRoI_Seeded:
        import copy
        tool_args = copy.deepcopy(kwargs)
        tool_args["RegSelTool"] = RegSelTool
        tool_args["RoIs"] = RoIs
        tool_args["isRoI_Seeded"] = isRoI_Seeded

    kwargs.setdefault("PixelDecodingTool", acc.popToolsAndMerge(
        ITkPixelDecodingToolCfg(flags,
            FailOnDecodingError = FailOnDecodingError,            
            ExpectedDataSize = ExpectedDataSize,
            **tool_args)))

    acc.addEventAlgo(CompFactory.ITkPixelDecodingAlg(name,
        pixelRDOKey = pixelRDOKey,
        RDOCacheKey = RDOCacheKey,
        **kwargs))

    return acc


def TrigITkPixelDecodingAlgCfg(flags, suffix, RoIs) -> ComponentAccumulator:
    """ Configures the ITk pixel raw data decoding with trigger (RoI-seeded, cached) args """
    acc = ComponentAccumulator()

    from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
    regSelTool = acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags))

    trigargs = {
        'RegSelTool' : regSelTool,
        'RoIs' : RoIs,
        'isRoI_Seeded' : True,
        'RDOCacheKey' : flags.Trigger.ITkTracking.PixRDOCacheKey,
    }

    acc.merge(ITkPixelDecodingAlgCfg(flags, name="ITkPixelDecodingAlg"+suffix, **trigargs))
    return acc

def ITkPixelDecodingPhaseIIRDOAlgCfg(flags, name = "ITkPixelDecodingPhaseIIRDOAlg",
        nRDOs = 1300000,
        pixelRDOKey = "ITkPixelRDOs",
        FailOnDecodingError = False,
        ExpectedDataSize = 2000000,
        RegSelTool = None,
        RoIs = None,
        isRoI_Seeded = False,
        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    tool_args = kwargs
    if isRoI_Seeded:
        import copy
        tool_args = copy.deepcopy(kwargs)
        tool_args["RegSelTool"] = RegSelTool
        tool_args["RoIs"] = RoIs
        tool_args["isRoI_Seeded"] = isRoI_Seeded

    kwargs.setdefault("PixelDecodingTool", acc.popToolsAndMerge(
        ITkPixelDecodingToolCfg(flags,
            FailOnDecodingError = FailOnDecodingError,            
            ExpectedDataSize = ExpectedDataSize,
            **tool_args)))

    acc.addEventAlgo(CompFactory.ITkPixelDecodingPhaseIIRDOAlg(name,
        pixelRDOKey = pixelRDOKey,
        nRDOs = nRDOs,
        **kwargs))

    return acc

def ITkPixelDecodingTracccCellsCUDAAlgCfg(flags,
        name = "ITkPixelDecodingTracccCellsCUDAAlg",
        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    acc.merge(addITkPixelCabling(flags))

    from AthCUDAServices.AthCUDAServicesConfig import (
        GPUSystemInfoSvcCfg, MemoryResourcesToolCfg, CopiesToolCfg, StreamToolCfg)
    acc.merge(GPUSystemInfoSvcCfg(flags))
    kwargs.setdefault("TracccCells", "TracccCells")
    kwargs.setdefault("MemoryResources", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopiesTool", acc.popToolsAndMerge(CopiesToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))
    kwargs.setdefault("GeoIdMappingObjectName", "TracccGeometryIdMapping")

    acc.addEventAlgo(CompFactory.ITkPixelDecodingTracccCellsCUDAAlg(name, **kwargs))

    return acc

def ITkPixelEncodingAlgCfg(flags, name = "ITkPixelEncodingAlg",
                           doMonitoring = False,
                           doExpertPlots = False,
                           **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    kwargs.setdefault("PixelConversionTool", acc.getPrimaryAndMerge( ITkPixelCnvToolCfg(flags,
                                                                                        doMonitoring = doMonitoring,
                                                                                        doExpertPlots = doExpertPlots,
                                                                                        **kwargs) ))

    acc.merge(addITkPixelCabling(flags))

    acc.addEventAlgo(CompFactory.ITkPixelEncodingAlg(name, **kwargs))

    return acc


def ITkPixelDataRateMonToolCfg(flags,
                               name = "ITkPixelDataRateMonTool",
                               doExpertPlots = False,
                               HistogramGroup: str="DataRateMon",
                               FileName: str='ITkPixelEncodingMonitoring.root') -> ComponentAccumulator:
    acc = ComponentAccumulator()

    histSvc = CompFactory.THistSvc(Output = [f"{HistogramGroup} DATAFILE='{FileName}', OPT='RECREATE'"] )
    acc.addService(histSvc)

    monitor = CompFactory.ITkPixelDataRateMonTool(name)
    monitor.HistSvc = histSvc
    monitor.DoExpertPlots = doExpertPlots

    acc.addPublicTool(monitor, primary=True)
    return acc


def ITkPixelCnvToolCfg(flags, name = "ITkPixelCnvTool",
                       doMonitoring = False,
                       doExpertPlots = False,
                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if doMonitoring:
        kwargs.setdefault("DataRateMonitoringTool", acc.getPrimaryAndMerge(ITkPixelDataRateMonToolCfg(flags, doExpertPlots=doExpertPlots)))

    kwargs.setdefault("HitSortingTool", acc.popToolsAndMerge(ITkPixelHitSortingToolCfg(name)))
    kwargs.setdefault("EncodingTool", acc.popToolsAndMerge(ITkPixelEncodingToolCfg(name)))
    kwargs.setdefault("PixelCablingKey", "ITkPixelCablingData")

    acc.addPublicTool(CompFactory.ITkPixelCnvTool(name, **kwargs), primary=True)
    return acc


def ITkPixelCsvWaferIdAlgCfg(flags,
                               name = "ITkPixelCsvWaferIdAlg",
                               OutputFile: str = "ITkPixelWaferIds.txt",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    kwargs.setdefault("OutputFile", OutputFile)
    acc.addEventAlgo(CompFactory.ITkPixelCsvWaferIdAlg(name, **kwargs))
    return acc

def PixelRDOValidationAlgCfg(flags, name="PixelRDOValidationAlg", **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.PixelRDOValidationAlg(name, **kwargs))
    return acc

def PixelPhaseIIRDOValidationAlgCfg(flags, name="PixelPhaseIIRDOValidationAlg", **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.PixelPhaseIIRDOValidationAlg(name, **kwargs))
    return acc

def PixelPhaseIIRDOVsPhaseIIRDOValidationAlgCfg(flags, name="PixelPhaseIIRDOVsPhaseIIRDOValidationAlg", **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.PixelPhaseIIRDOVsPhaseIIRDOValidationAlg(name, **kwargs))
    return acc
