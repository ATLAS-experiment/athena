#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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

def ITkPixelDecodingAlgCfg(flags, name = "ITkPixelDecodingAlg", **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    acc.addEventAlgo(CompFactory.ITkPixelDecodingAlg(name, **kwargs))

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
