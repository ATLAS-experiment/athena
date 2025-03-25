# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Configuration of tools shared between Segment Finding and Track Building

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def MuonEDMPrinterToolCfg(flags, name="MuonEDMPrinterTool", **kwargs):
    result = ComponentAccumulator()  
    kwargs.setdefault('TgcPrdCollection', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    #kwargs.setdefault('TgcPrdCollection', 'TGC_Measurements' )
    # We need to override TgcPrdCollection to match old config, so keep line above for ease of testing.

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))

    the_tool = CompFactory.Muon.MuonEDMPrinterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    result.merge(MuonEDMHelperSvcCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg   
    result.merge(MuonIdHelperSvcCfg(flags))
    return result
    
def MuonEDMHelperSvcCfg(flags, name = "MuonEDMHelperSvc", **kwargs):
    result = ComponentAccumulator()
    the_svc = CompFactory.Muon.MuonEDMHelperSvc(name, **kwargs)
    result.addService(the_svc, primary = True)
    return result
       
def MuonTrackToSegmentToolCfg(flags,name="MuonTrackToSegmentTool", **kwargs):
    Muon__MuonTrackToSegmentTool=CompFactory.Muon.MuonTrackToSegmentTool
    #MDT conditions information not available online
    result = ComponentAccumulator()
    from MuonConfig.MuonCondAlgConfig import MuonStationIntersectCondAlgCfg
    result.merge(MuonStationIntersectCondAlgCfg(flags))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    atlasRungeKuttaPropagator = result.popToolsAndMerge(RungeKuttaPropagatorCfg(flags))
    result.addPublicTool(atlasRungeKuttaPropagator)
    kwargs.setdefault("Propagator",atlasRungeKuttaPropagator)
    
    # Not bothering to explicitly set IdHelper or EDMHelper    
    muon_track_to_segment_tool = Muon__MuonTrackToSegmentTool(name, **kwargs)
    result.setPrivateTools(muon_track_to_segment_tool)
    return result

def MuonHitSummaryToolCfg(flags, name="MuonHitSummaryTool", **kwargs):
    result = ComponentAccumulator()       
    kwargs.setdefault('MuonTrackSummaryHelperTool', result.popToolsAndMerge(MuonTrackSummaryHelperToolCfg(flags)))
    printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault('Printer', printer )
    result.addPublicTool(printer)
    the_tool = CompFactory.Muon.MuonHitSummaryTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonSeededSegmentFinderCfg(flags,name="MuonSeededSegmentFinder", **kwargs):
    Muon__MuonSeededSegmentFinder=CompFactory.Muon.MuonSeededSegmentFinder
    from MuonConfig.MuonSegmentFindingConfig import DCMathSegmentMakerCfg, MdtMathSegmentFinderCfg # FIXME - should really shift this to RecTools then.
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg

    result = ComponentAccumulator()
    
    if "SegmentMaker" not in kwargs or "SegmentMakerNoHoles" not in kwargs:
        seg_maker=""
        mdt_segment_finder = result.popToolsAndMerge( MdtMathSegmentFinderCfg(flags, name="MCTBMdtMathSegmentFinder", UseChamberTheta = False, AssociationRoadWidth = 1.5) )
        if flags.Beam.Type is BeamType.Collisions:
            seg_maker = result.popToolsAndMerge ( DCMathSegmentMakerCfg( flags, name = "MCTBDCMathSegmentMaker", MdtSegmentFinder = mdt_segment_finder, SinAngleCut = 0.04, DoGeometry = True))
        else:  # cosmics or singlebeam
            seg_maker = result.popToolsAndMerge (DCMathSegmentMakerCfg( flags, name = "MCTBDCMathSegmentMaker", MdtSegmentFinder = mdt_segment_finder, SinAngleCut = 0.1,  DoGeometry = False, AddUnassociatedPhiHits= True ))
        kwargs.setdefault("SegmentMaker", seg_maker)
        kwargs.setdefault("SegmentMakerNoHoles", seg_maker) #FIXME. Just remove one.
    
    kwargs.setdefault("Propagator", result.popToolsAndMerge(RungeKuttaPropagatorCfg(flags)) )
    kwargs.setdefault("MdtRotCreator", result.popToolsAndMerge (MdtDriftCircleOnTrackCreatorCfg(flags)))
    kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) ) # private here
    if not flags.Detector.GeometryCSC:
        kwargs.setdefault("CscPrepDataContainer","")
    if not flags.Detector.GeometrysTGC:
        kwargs.setdefault("sTgcPrepDataContainer","")
    if not flags.Detector.GeometryMM:
        kwargs.setdefault("MMPrepDataContainer","")
    
  
    kwargs.setdefault('TgcPrepDataContainer', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    
    muon_seeded_segment_finder = Muon__MuonSeededSegmentFinder(name, **kwargs)
    result.setPrivateTools(muon_seeded_segment_finder)
    return result
        
        
def MuonSegmentMomentumFromFieldCfg(flags, name="MuonSegmentMomentumFromField", **kwargs):
    MuonSegmentMomentumFromField=CompFactory.MuonSegmentMomentumFromField
    result = ComponentAccumulator()
    
    from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasNavigatorCfg
    navigator = result.popToolsAndMerge (AtlasNavigatorCfg(flags, name='InDetNavigator'))
    kwargs.setdefault("NavigatorTool", navigator)
    
    from TrkConfig.TrkExSTEP_PropagatorConfig import AtlasSTEP_PropagatorCfg
    muon_prop =  result.popToolsAndMerge(AtlasSTEP_PropagatorCfg(flags))
    kwargs.setdefault("PropagatorTool", muon_prop)
        
    muon_seg_mom_from_field = MuonSegmentMomentumFromField(name=name, **kwargs)
    result.setPrivateTools(muon_seg_mom_from_field)
    return result
    
def MuonTrackSummaryHelperToolCfg(flags, name="MuonTrackSummaryHelperTool", **kwargs):
    result = ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg, MuonStraightLineExtrapolatorCfg
    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(AtlasExtrapolatorCfg(flags)) )
    kwargs.setdefault("StaightLineExtrapolator", result.popToolsAndMerge(MuonStraightLineExtrapolatorCfg(flags)) )
    kwargs.setdefault("CalculateCloseHits", True)
    result.setPrivateTools(CompFactory.Muon.MuonTrackSummaryHelperTool(name=name,**kwargs))
    return result

def MuonTrackScoringToolCfg(flags, name="MuonTrackScoringTool", **kwargs):
    Muon__MuonTrackScoringTool=CompFactory.Muon.MuonTrackScoringTool
    # m_trkSummaryTool("Trk::TrackSummaryTool"),    
    result = ComponentAccumulator()
    from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
    track_summary = result.getPrimaryAndMerge(MuonTrackSummaryToolCfg(flags)) 
    kwargs.setdefault('SumHelpTool', track_summary)
    printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) 
    kwargs.setdefault("EDMPrinter", printer)
    result.setPrivateTools(Muon__MuonTrackScoringTool(name=name,**kwargs))
    return result

def MuonAmbiProcessorCfg(flags, name="MuonAmbiProcessor", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('DropDouble', False)
    scoring_tool =  result.getPrimaryAndMerge(MuonTrackScoringToolCfg( flags ))
    kwargs.setdefault('ScoringTool', scoring_tool )
    result.addPublicTool(scoring_tool)
    muon_edm_printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg( flags )) #private here
    # muon_edm_printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg( flags, TgcPrdCollection="TGC_Measurements" )) # FIXME Hack to get wrapping working. Keep in for now, to aid debugging

    muon_ami_selection_tool = CompFactory.Muon.MuonAmbiTrackSelectionTool(name="MuonAmbiSelectionTool", Printer=muon_edm_printer)
    result.addPublicTool(muon_ami_selection_tool)
    kwargs.setdefault('SelectionTool', muon_ami_selection_tool)
    result.setPrivateTools(CompFactory.Trk.TrackSelectionProcessorTool(name=name,**kwargs))
    return result

def MuonTrackCleanerCfg(flags, name="MuonTrackCleaner", seg=False, **kwargs):
    Muon__MuonTrackCleaner=CompFactory.Muon.MuonTrackCleaner
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg, TriggerChamberClusterOnTrackCreatorCfg
    from TrkConfig.AtlasExtrapolatorConfig import MuonStraightLineExtrapolatorCfg

    result=ComponentAccumulator()
    
    mdt_creator= result.getPrimaryAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags))
    kwargs.setdefault("MdtRotCreator", mdt_creator)
    
    muon_cluster_creator= result.getPrimaryAndMerge(TriggerChamberClusterOnTrackCreatorCfg(flags))
    kwargs.setdefault("CompRotCreator", muon_cluster_creator)
    
    # For PullCalculator, just let it get default for moment.
    
    extrapolator = result.getPrimaryAndMerge(MuonStraightLineExtrapolatorCfg(flags)) 
    kwargs.setdefault("Extrapolator", extrapolator)

    from TrkConfig.TrkGlobalChi2FitterConfig import MCTBSLFitterMaterialFromTrackCfg, MCTBFitterMaterialFromTrackCfg
    slfitter = result.popToolsAndMerge(MCTBSLFitterMaterialFromTrackCfg(flags))
    kwargs.setdefault("SLFitter", slfitter)

    if "PullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("PullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))
    
    if seg:
        # I might move this into its own function eventually
        # See ATLASRECTS-7325 for more discussion of this
        kwargs.setdefault("Fitter", result.popToolsAndMerge(MCTBSLFitterMaterialFromTrackCfg(flags)))
        kwargs.setdefault("PullCut", 3)
        kwargs.setdefault("PullCutPhi", 3)
        kwargs.setdefault("UseSLFit", True)
    else:
        kwargs.setdefault("Fitter", result.popToolsAndMerge(MCTBFitterMaterialFromTrackCfg(flags)))

    kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) ) #private here
    # kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags, TgcPrdCollection="TGC_Measurements" )) ) # FIXME Hack to get wrapping working. Keep in for now, to aid debugging

    kwargs.setdefault("MaxAvePullSumPerChamber", 6)
    kwargs.setdefault("Chi2Cut", flags.Muon.Chi2NDofCut)
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("Iterate", False)
        kwargs.setdefault("RecoverOutliers", False)
    
    result.setPrivateTools(Muon__MuonTrackCleaner(name, **kwargs))
    
    return result 

def MuonPhiHitSelectorCfg(flags, name="MuonPhiHitSelector",**kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg

    kwargs.setdefault("MakeClusters", True)
    kwargs.setdefault("CompetingRios", True)
    kwargs.setdefault("DoCosmics", flags.Beam.Type is BeamType.Cosmics)
    result=MuonClusterOnTrackCreatorCfg(flags)
    cluster_creator = result.popPrivateTools()
    kwargs.setdefault("MuonCompetingClustersOnTrackCreator", CompFactory.Muon.MuonCompetingClustersOnTrackCreator(name='MuonCompetingClustersOnTrackCreator', ClusterCreator=cluster_creator) )
    kwargs.setdefault("MuonClusterOnTrackCreator", cluster_creator )
    result.setPrivateTools(CompFactory.MuonPhiHitSelector(name,**kwargs))
    return result


def MuPatHitToolCfg(flags, name="MuPatHitTool",**kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    
    result = RungeKuttaPropagatorCfg(flags)
    kwargs.setdefault("AtlasRungeKuttaPropagator",result.popPrivateTools())
    kwargs.setdefault("MdtRotCreator", 
        result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags, name = "MdtDriftCircleOnTrackCreatorPreFit", DoFixedError = True, CreateTubeHit = True, DoSegmentErrors = False)) )

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))
    
    if flags.Detector.GeometryCSC:
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import CscClusterOnTrackCreatorCfg
        kwargs.setdefault("CscRotCreator", result.popToolsAndMerge(CscClusterOnTrackCreatorCfg(flags)))
    else:
        kwargs.setdefault("CscRotCreator", "")    
    printer =  result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault('Printer', printer)
    result.addPublicTool( printer )
    kwargs.setdefault('edmHelper', result.getPrimaryAndMerge(MuonEDMHelperSvcCfg(flags)))
    kwargs.setdefault("isCosmic", flags.Beam.Type is BeamType.Cosmics)

    result.setPrivateTools(CompFactory.Muon.MuPatHitTool(name,**kwargs))
    return result


def MuonTrackExtrapolationToolCfg(flags, name="MuonTrackExtrapolationTool", **kwargs):
    # FIXME - it seems like this tool needs a lot of configuration still.
    # But perhaps it can be simplified first?
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg, MuonExtrapolatorCfg
    result = ComponentAccumulator()
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    condAlg = result.getPrimaryAndMerge(TrackingGeometryCondAlgCfg(flags))
    geom_cond_key = condAlg.TrackingGeometryWriteKey
    kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)

    kwargs.setdefault("AtlasExtrapolator", result.popToolsAndMerge( AtlasExtrapolatorCfg(flags) ) )
    kwargs.setdefault("MuonExtrapolator",  result.popToolsAndMerge( MuonExtrapolatorCfg(flags) ) )
    kwargs.setdefault('EDMPrinter', result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))) #private here
    result.setPrivateTools(
        CompFactory.Muon.MuonTrackExtrapolationTool(name, **kwargs))
    return result

def MuonRefitToolCfg(flags, name="MuonRefitTool", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg, TriggerChamberClusterOnTrackCreatorCfg
    from TrkConfig.TrkGlobalChi2FitterConfig import MCTBFitterMaterialFromTrackCfg
    from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg

    result = ComponentAccumulator()
    if flags.IOVDb.DatabaseInstance == 'COMP200' or \
                'HLT'  in flags.IOVDb.GlobalTag or flags.Common.isOnline or flags.Muon.MuonTrigger:
        kwargs["AlignmentErrorTool"] = None
    else:
        from MuonAlignErrorTool.AlignmentErrorToolConfig import AlignmentErrorToolCfg
        kwargs.setdefault("AlignmentErrorTool", result.popToolsAndMerge(AlignmentErrorToolCfg(flags)))
    printer =  result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault('Printer', printer) #PublicToolHandle
    result.addPublicTool(printer)
    kwargs.setdefault("Fitter", result.popToolsAndMerge(MCTBFitterMaterialFromTrackCfg(flags)))
    kwargs.setdefault("MuonExtrapolator", result.popToolsAndMerge( MuonExtrapolatorCfg(flags) ) )
    kwargs.setdefault("MdtRotCreator", result.popToolsAndMerge( MdtDriftCircleOnTrackCreatorCfg(flags) ) )
    kwargs.setdefault("CompClusterCreator", result.popToolsAndMerge( TriggerChamberClusterOnTrackCreatorCfg(flags) ) )
    # T0Fitter is null by default
    kwargs.setdefault("MuonEntryExtrapolationTool", result.popToolsAndMerge(MuonTrackExtrapolationToolCfg(flags)) )

    result.setPrivateTools(CompFactory.Muon.MuonRefitTool(name, **kwargs))
    return result

def SimpleMMClusterBuilderToolCfg(flags, name = "SimpleMMClusterBuilderTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))
    the_tool = CompFactory.Muon.SimpleMMClusterBuilderTool(name,**kwargs)
    result.setPrivateTools(the_tool)   
    return result

def UTPCMMClusterBuilderToolCfg(flags, name = "UTPCMMClusterBuilderTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))
    the_tool = CompFactory.Muon.UTPCMMClusterBuilderTool(name,**kwargs)
    result.setPrivateTools(the_tool)   
    return result

def ClusterTimeProjectionMMClusterBuilderToolCfg(flags, name = "ClusterTimeProjectionMMClusterBuilderTool", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonCalibrationConfig import NswErrorCalibDbAlgCfg
    result.merge(NswErrorCalibDbAlgCfg(flags))  

    #Turn on effective drift velocity calibration for data by default
    from MuonConfig.MuonConfigFlags import MMClusterBuilderEnum
    if not flags.Input.isMC and flags.Muon.MMClusterCalibRecoTool == MMClusterBuilderEnum.ClusterTimeProjection:
      from MuonConfig.MuonCalibrationConfig import MmCTPCondDbAlgCfg
      result.merge(MmCTPCondDbAlgCfg(flags))

    the_tool = CompFactory.Muon.ClusterTimeProjectionMMClusterBuilderTool(name,**kwargs)
    result.setPrivateTools(the_tool)   
    return result
  
def SimpleSTgcClusterBuilderToolCfg(flags, name = "SimpleSTgcClusterBuilderTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.Muon.SimpleSTgcClusterBuilderTool(name,**kwargs)
    result.setPrivateTools(the_tool) 
    return result

def CaruanaSTgcClusterBuilderToolCfg(flags, name = "CaruanaSTgcClusterBuilderTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.Muon.CaruanaSTgcClusterBuilderTool(name,**kwargs)
    result.setPrivateTools(the_tool) 
    return result
