# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SegmentSelectorCfg(flags, name="SegmentSelectionTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonR4.SegmentSelectionTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MSTrackFitterCfg(flags, name="MSTrackFitTool", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsConfigFlags import TrackFitterType
    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("fitterKind", TrackFitterType.GlobalChiSquareFitter)
    kwargs.setdefault("OutlierChi2Cut", 200000)
    kwargs.setdefault("DoReFitFromPRD", False)
    kwargs.setdefault("IncludeScattering", False)
    kwargs.setdefault("IncludeELoss", False)
    
    kwargs.setdefault("MaxPropagationStep", 1000000)
    kwargs.setdefault("MaxSurfacesPerNavStep", 10000000)
    kwargs.setdefault("DoFreeToBoundCorrection", True)
    kwargs.setdefault("MaxIterations", 100)
    

    kwargs.setdefault("MuonCalibrationTool",result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    fitTool = result.popToolsAndMerge(ActsFitterCfg(flags, name=name, **kwargs))
    result.setPrivateTools(fitTool)
    return result

def TrackSummaryToolCfg(flags, name="MuonTrackSummaryTool", **kwargs) :
    result = ComponentAccumulator()
    theTool = CompFactory.MuonR4.TrackSummaryTool(name, **kwargs)
    result.setPrivateTools(theTool)
    return result

def MSTrackFinderAlgCfg(flags, name="MSTrackFinderAlg", **kwargs):
    result = ComponentAccumulator()
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
 
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags)))       
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
 

    the_alg = CompFactory.MuonR4.MsTrackFindingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    ###

    return result