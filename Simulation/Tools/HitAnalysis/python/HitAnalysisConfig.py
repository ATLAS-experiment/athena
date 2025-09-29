# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HitAnalysisOutputCfg(flags, output_name='SiHitAnalysis'):
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    return setupHistSvcCfg(flags, outFile=flags.Output.HISTFileName, outStream=output_name)


def BLMHitAnalysisCfg(flags, name='BLMHitAnalysis', **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
    acc = PixelSimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'BLM_Hits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def BCMHitAnalysisCfg(flags, name='BCMHitAnalysis', **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
    acc = PixelSimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'BCM_Hits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def PixelHitAnalysisCfg(flags, name='PixelHitAnalysis', **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
    acc = PixelSimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'PixelHits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def SCTHitAnalysisCfg(flags, name='SCTHitAnalysis', **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_SimulationGeometryCfg
    acc = SCT_SimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'SCT_Hits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def TRTHitAnalysisCfg(flags, name='TRTHitAnalysis', **kwargs):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_SimulationGeometryCfg
    acc = TRT_SimulationGeometryCfg(flags)

    kwargs.setdefault('HistPath', '/TRTHitAnalysis/')
    acc.addEventAlgo(CompFactory.TRTHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags, output_name="TRTHitAnalysis"))

    return acc


def ITkPixelHitAnalysisCfg(flags, name='ITkPixelHitAnalysis', **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelSimulationGeometryCfg
    acc = ITkPixelSimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'ITkPixelHits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/histos/')
    kwargs.setdefault('NtuplePath', '/SiHitAnalysis/ntuples/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def ITkStripHitAnalysisCfg(flags, name='ITkStripHitAnalysis', **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripSimulationGeometryCfg
    acc = ITkStripSimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'ITkStripHits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/histos/')
    kwargs.setdefault('NtuplePath', '/SiHitAnalysis/ntuples/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def HGTD_HitAnalysisCfg(flags, name='HGTD_HitAnalysis', **kwargs):
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg
    acc = HGTD_SimulationGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'HGTD_Hits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/histos/')
    kwargs.setdefault('NtuplePath', '/SiHitAnalysis/ntuples/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def PLR_HitAnalysisCfg(flags, name='PLR_HitAnalysis', **kwargs):
    from PLRGeoModelXml.PLR_GeoModelConfig import PLR_GeometryCfg
    acc = PLR_GeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'PLR_Hits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/histos/')
    kwargs.setdefault('NtuplePath', '/SiHitAnalysis/ntuples/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def BCMPrimeHitAnalysisCfg(flags, name='BCMPrimeHitAnalysis', **kwargs):
    from BCMPrimeGeoModelXml.BCMPrimeGeoModelConfig import BCMPrimeGeometryCfg
    acc = BCMPrimeGeometryCfg(flags)

    kwargs.setdefault('CollectionName', 'BCMPrimeHits')
    kwargs.setdefault('HistPath', '/SiHitAnalysis/histos/')
    kwargs.setdefault('NtuplePath', '/SiHitAnalysis/ntuples/')
    acc.addEventAlgo(CompFactory.SiHitAnalysis(name, **kwargs))

    acc.merge(HitAnalysisOutputCfg(flags))

    return acc


def SiHitAnalysisCfg(flags):
    acc = ComponentAccumulator()

    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelHitAnalysisCfg(flags))

    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripHitAnalysisCfg(flags))

    if flags.Detector.EnableHGTD:
        acc.merge(HGTD_HitAnalysisCfg(flags))

    if flags.Detector.EnablePLR:
        acc.merge(PLR_HitAnalysisCfg(flags))

    if flags.Detector.EnableBCMPrime:
        acc.merge(BCMPrimeHitAnalysisCfg(flags))

    return acc


# Calorimeter
def CaloHitAnalysisCfg(flags, name='CaloHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        acc.merge(LArGMCfg(flags))
    kwargs.setdefault("UseLAr", flags.Detector.GeometryLAr)

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        acc.merge(TileGMCfg(flags))
    kwargs.setdefault("UseTile", flags.Detector.GeometryTile)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.CaloHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name=name))
    return acc


# Muon System
def RPCHitAnalysisCfg(flags, name='RPCHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.RPCHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="RPCHitAnalysis"))

    return acc


def MDTHitAnalysisCfg(flags, name='MDTHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.MDTHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="MDTHitAnalysis"))

    return acc


def CSCHitAnalysisCfg(flags, name='CSCHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.CSCHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="CSCHitAnalysis"))

    return acc


def TGCHitAnalysisCfg(flags, name='TGCHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.TGCHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="TGCHitAnalysis"))

    return acc


def MMHitAnalysisCfg(flags, name='MMHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.MMHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="MMHitAnalysis"))

    return acc

def xMuonHitAnalysisCfg(flags, **kwargs):
    result = ComponentAccumulator()
    if not flags.Muon.usePhaseIIGeoSetup:
        return result
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.addEventAlgo(CompFactory.MuonValR4.xMuonHitAnalysis(**kwargs))
    histPath = kwargs["HistPath"]
    result.merge(HitAnalysisOutputCfg(flags, output_name=histPath[ : histPath.rfind("/")]))
    return result


def sTGCHitAnalysisCfg(flags, name='sTGCHitAnalysis', **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc = MuonGeoModelCfg(flags)

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.sTGCHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="sTGCHitAnalysis"))

    return acc


# Forward
def ALFAHitAnalysisCfg(flags, name='ALFAHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.ALFAHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="ALFAHitAnalysis"))

    return acc


def AFPHitAnalysisCfg(flags, name='AFPHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.AFPHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="AFPHitAnalysis"))

    return acc


def LucidHitAnalysisCfg(flags, name='LucidHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.LucidHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="LucidHitAnalysis"))

    return acc


def ZDCHitAnalysisCfg(flags, name='ZDCHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.ZDCHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="ZDCHitAnalysis"))

    return acc


#Truth
def TrackRecordAnalysisCfg(flags, name='TrackRecordAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.TrackRecordAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="TrackRecordAnalysis"))

    return acc


def TruthHitAnalysisCfg(flags, name='TruthHitAnalysis', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('HistPath', f'/{name}/')
    acc.addEventAlgo(CompFactory.TruthHitAnalysis(name, **kwargs))
    acc.merge(HitAnalysisOutputCfg(flags, output_name="TruthHitAnalysis"))

    return acc
