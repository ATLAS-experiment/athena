# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This file configures the Muon segment finding. It is based on a few files in the old configuration system:
# Tools, which are configured here: 
# https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecExample/python/MuonRecTools.py
# https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecExample/python/MooreTools.py
# from https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecExample/python/CscTools.py

# Core
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType, Format

#Local
from TrkConfig.TrkGlobalChi2FitterConfig import MCTBFitterCfg, MCTBSLFitterCfg, MCTBSLFitterMaterialFromTrackCfg
from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg, MuonTrackCleanerCfg, MuonEDMPrinterToolCfg
from MuonConfig.MuonCondAlgConfig import MuonStationIntersectCondAlgCfg
from TrkConfig.AtlasExtrapolatorConfig import MuonStraightLineExtrapolatorCfg
from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtCalibWindowNumber


def MuonCombinePatternToolCfg(flags, name="MuonCombinePatternTool",**kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("UseTightAssociation",flags.Beam.Type is BeamType.Collisions)
    kwargs.setdefault("UseCosmics", flags.Beam.Type is not BeamType.Collisions )
    the_tool = CompFactory.MuonCombinePatternTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonHoughPatternToolCfg(flags, name = "MuonHoughPatternTool",**kwargs):
    result = ComponentAccumulator()
    if flags.Beam.Type is not BeamType.Collisions:
         kwargs.setdefault("UseCosmics", True)
         kwargs.setdefault("NumberOfMaximaPerIterations", 1)
    the_tool = CompFactory.MuonHoughPatternTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result
def MuonHoughPatternFinderToolCfg(flags, name = "MuonHoughPatternFinderTool", **kwargs):
    result = ComponentAccumulator()
    if flags.Beam.Type is BeamType.Collisions:
        kwargs.setdefault("MDT_TDC_cut", False)
        kwargs.setdefault("RecordAll",False)
    kwargs.setdefault("muonHoughPatternTool", result.popToolsAndMerge(MuonHoughPatternToolCfg(flags)) )
    kwargs.setdefault("muonCombinePatternTool", result.popToolsAndMerge(MuonCombinePatternToolCfg(flags)) )
    kwargs.setdefault("DoSummary", flags.Muon.printSummary)
    the_tool = CompFactory.Muon.MuonHoughPatternFinderTool(name,**kwargs)
    result.setPrivateTools(the_tool)
    return result

def MdtDriftCircleOnTrackCreatorAdjustableT0Cfg(flags,**kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg
    kwargs.setdefault("TimingMode", 3)
    kwargs.setdefault("DoTofCorrection", True)
    kwargs.setdefault("TimeWindowSetting", MdtCalibWindowNumber('Collision_data'))
    acc = MuonClusterOnTrackCreatorCfg(flags, **kwargs)  
    return acc

def AdjustableT0Tool(flags,**kwargs):
    # NB: the following 'ifs' are the same as in the MdtDriftCircleOnTrackCreator, so that the ToF setting is the same
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("DoTof", 0)
    else: # collisions simulation final precise cuts
        kwargs.setdefault("DoTof", 1)
        
    AdjT0__AdjustableT0Tool=CompFactory.getComp("AdjT0::AdjustableT0Tool")
    return AdjT0__AdjustableT0Tool(**kwargs)

def MdtMathSegmentFinderCfg(flags,name="MdtMathSegmentFinder", **kwargs):
    # beamType       = getattr(extraFlags,"beamType", beamFlags.beamType())
    # enableCurvedSegmentFinding = getattr(extraFlags,"enableCurvedSegmentFinding", muonStandaloneFlags.enableCurvedSegmentFinding())
    result = ComponentAccumulator()
    
    doSegmentT0Fit = kwargs.pop('doSegmentT0Fit', flags.Muon.doSegmentT0Fit)

    kwargs.setdefault("FinderDebugLevel", 0) # This is just to avoid that the tool seems unconfigured. Real fix is to use default name.

    if doSegmentT0Fit and not (flags.Muon.MuonTrigger and flags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics]):
        kwargs.setdefault("AssociationRoadWidth", 3.)
        kwargs.setdefault("MDTAssocationPullcut", 3.)
        kwargs.setdefault("RecoverMdtOutliers", False)
        kwargs.setdefault("DCFitProvider", result.popToolsAndMerge(MdtSegmentT0FitterCfg(flags) ) )

    if flags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics] or flags.Input.isMC is False:
        kwargs.setdefault("AssociationRoadWidth", 2.)
        kwargs.setdefault("MDTAssocationPullcut", 4.)
        kwargs.setdefault("RecoverMdtOutliers", True )
        

    if flags.Muon.enableCurvedSegmentFinding:
        kwargs.setdefault("DoCurvedSegmentFinder",True)
    result.setPrivateTools(CompFactory.Muon.MdtMathSegmentFinder(name=name,**kwargs))
    return result

def MuonSegmentFittingToolCfg(flags, name = "MuonSegmentFittingTool",  **kwargs):
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    result = RungeKuttaPropagatorCfg(flags, name='AtlasRungeKuttaPropagator')
    kwargs.setdefault("SLPropagator", 
                      result.popPrivateTools())
    kwargs.setdefault("SLFitter", 
                      result.popToolsAndMerge(MCTBSLFitterCfg(flags)))
    kwargs.setdefault("CurvedFitter", 
                      result.popToolsAndMerge(MCTBFitterCfg(flags)))
    kwargs.setdefault("TrackCleaner", result.popToolsAndMerge(MuonTrackCleanerCfg(flags)))
  
    result.setPrivateTools(CompFactory.Muon.MuonSegmentFittingTool(name,**kwargs))    
    return result

def MdtSegmentT0FitterCfg(flags, name="MdtSegmentT0Fitter", **kwargs):
    result = ComponentAccumulator()    
    kwargs.setdefault("FloatSegDirection", flags.Beam.Type is BeamType.Cosmics)
    result.setPrivateTools(CompFactory.TrkDriftCircleMath.MdtSegmentT0Fitter(name, **kwargs))    
    return result

def DCMathSegmentMakerCfg(flags,
                          name='DCMathSegmentMaker',
                          doSegmentT0Fit=False,
                          **kwargs):
    
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg, TriggerChamberClusterOnTrackCreatorCfg, MdtCalibToolForRotsCfg
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSegmentSelectionToolCfg
    
    # This in general is a pretty problematic piece of code. It seems to have a lot of potential issues, because it has loads of mutables / subtools etc
    # https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonSegmentMakers/MuonSegmentMakerTools/DCMathSegmentMaker/src/DCMathSegmentMaker.h
    # ToolHandle<IMdtDriftCircleOnTrackCreator> m_mdtCreator;         //<! mdt rio ontrack creator
    # ToolHandle<IMdtDriftCircleOnTrackCreator> m_mdtCreatorT0;       //<! mdt rio ontrack creator
    # ToolHandle<IMuonClusterOnTrackCreator>    m_clusterCreator;     //<! cluster rio ontrack creator
    # ToolHandle<IMuonCompetingClustersOnTrackCreator> m_compClusterCreator;   //<! competing clusters rio ontrack creator
    # ToolHandle<MuonEDMPrinterTool>            m_printer;         //<! printer helper tool
    # ServiceHandle<IMuonEDMHelperSvc>          m_edmHelperSvc;          //<! printer helper tool
    # ToolHandle<IMdtSegmentFinder>             m_segmentFinder;   //<! segment finder tool MdtSegmentFinder
    # ToolHandle<IMuonSegmentFittingTool>       m_segmentFitter;   //<! segment fitting tool
    # ToolHandle<IMuonSegmentSelectionTool>     m_segmentSelectionTool; //<! segment selection tool
    # ToolHandle<IDCSLFitProvider>              m_dcslFitProvider;

    result=ComponentAccumulator()
    # Won't explicitly configure MuonIdHelperSvc
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg
    kwargs.setdefault("edmHelper", result.getPrimaryAndMerge(MuonEDMHelperSvcCfg(flags)))
    kwargs.setdefault("RefitSegment", True)
    kwargs.setdefault("AssumePointingPhi", flags.Beam.Type is not BeamType.Cosmics)
    kwargs.setdefault("OutputFittedT0", True)

    kwargs.setdefault("DCFitProvider", result.popToolsAndMerge(MdtSegmentT0FitterCfg(flags)))
    #kwargs.setdefault("CurvedErrorScaling", False)
    kwargs.setdefault("UsePreciseError", True)
    kwargs.setdefault("SinAngleCut", 0.4)

    doSegmentT0Fit =  doSegmentT0Fit or flags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics]
    if flags.Beam.Type in [BeamType.SingleBeam, BeamType.Cosmics]:
        kwargs.setdefault("SinAngleCut", 0.9)
        kwargs.setdefault("AddUnassociatedPhiHits", True)
        kwargs.setdefault("RecoverBadRpcCabling", True)
        kwargs.setdefault("CurvedErrorScaling", False)
    elif not flags.Input.isMC: # collisions real data 
        kwargs.setdefault("AddUnassociatedPhiHits", True)
        kwargs.setdefault("RecoverBadRpcCabling", True)

    if flags.Muon.updateSegmentSecondCoordinate:
        kwargs.setdefault("UpdatePhiUsingPhiHits",True)

    if flags.Muon.enableCurvedSegmentFinding:
        kwargs.setdefault("CurvedErrorScaling", False)
        kwargs.setdefault("PreciseErrorScale", 1)
        kwargs.setdefault("UsePreciseError", True)
    
    # Now stuff that wasn't explicitly configured before.
    
    result.merge(MuonStationIntersectCondAlgCfg(flags))

    kwargs.setdefault("MdtCreator", result.getPrimaryAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags)))
    # Set MdtCreatorT0 below
    kwargs.setdefault("MuonCompetingClustersCreator", result.getPrimaryAndMerge(TriggerChamberClusterOnTrackCreatorCfg(flags)))
    edm_printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags) ) # Needed again below
    kwargs.setdefault("EDMPrinter", edm_printer )
    if doSegmentT0Fit:        
        mdt_dcot_CA = MdtDriftCircleOnTrackCreatorCfg(flags,    
                                                      name="MdtDriftCircleOnTrackCreatorAdjustableT0", 
                                                      TimingMode=3,
                                                      CalibrationTool = result.popToolsAndMerge(MdtCalibToolForRotsCfg(flags, 
                                                                                                                       DoTofCorrection=True, 
                                                                                                                       TimeWindowSetting=MdtCalibWindowNumber('Collision_data'))))
        kwargs.setdefault("MdtCreatorT0", result.getPrimaryAndMerge(mdt_dcot_CA)) 
        kwargs.setdefault("MdtSegmentFinder", result.getPrimaryAndMerge(MdtMathSegmentFinderCfg(flags, name='MdtMathT0FitSegmentFinder', doSegmentT0Fit=True)))
    else:
        kwargs.setdefault("MdtCreatorT0", "") 
        kwargs.setdefault("MdtSegmentFinder", result.getPrimaryAndMerge(MdtMathSegmentFinderCfg(flags)) )

    kwargs.setdefault("SegmentFitter", result.getPrimaryAndMerge(MuonSegmentFittingToolCfg(flags, name="MuonSegmentFittingTool")))
    
    kwargs.setdefault("SegmentSelector", result.popToolsAndMerge(MuonSegmentSelectionToolCfg(flags)))
   
    dc_segment_maker = CompFactory.Muon.DCMathSegmentMaker(name=name, **kwargs)
    result.setPrivateTools(dc_segment_maker)
    return result


def CscAlignmentTool(flags, **kwargs):

    CscAlignmentTool=CompFactory.CscAlignmentTool
    etaposAlignConsts = [ # 1st, 2nd, 3rd, 4th layer 
        0.0,   -0.0902347,   -0.0984321,    -0.141175, #sector -16
        0.0,    -0.166412,    -0.150399,     -0.18592, #sector -15
        0.0,   -0.0544449,     0.101448,   -0.0433321, #sector -14
        0.0,    -0.047769,    -0.156247,   -0.0854826, #sector -13
        0.0,   -0.0970101,    -0.227578,    -0.301079, #sector -12
        0.0,    -0.152912,    -0.365218,    -0.314042, #sector -11
        0.0,    0.0338696,    -0.264766,    -0.224567, #sector -10
        0.0,    -0.107372,     -0.16099,     -0.13508, #sector -9
        0.0,   -0.0663979,   -0.0312645,    -0.130711, #sector -8
        0.0,    -0.249974,    -0.269372,    -0.353648, #sector -7
        0.0,    -0.194175,    0.0238348,    -0.188433, #sector -6
        0.0,    0.0977346,    0.0492461,     0.150434, #sector -5
        0.0,    0.0444637,   -0.0871214,   -0.0627541, #sector -4
        0.0,    0.0347245,   -0.0226186,   -0.0826478, #sector -3
        0.0,  -0.00408879,   -0.0638005,    -0.206868, #sector -2
        0.0,    0.0331328,    0.0402103,     0.118611, #sector -1
        0.0,   -0.0634537,   -0.0516743,    0.0045364, #sector 1
        0.0,   -0.0957718,    -0.235246,    -0.439811, #sector 2
        0.0,    -0.163504,   -0.0129115,   0.00140143, #sector 3
        0.0,    -0.182903,   -0.0503858,    -0.207799, #sector 4
        0.0,   -0.0970927,            0,     0.124744, #sector 5
        0.0,   -0.0169602,   -0.0527447,    0.0319154, #sector 6
        0.0,    -0.176499,    -0.196542,   -0.0846979, #sector 7
        0.0,    0.0589994,    0.0131431,    0.0461769, #sector 8
        0.0,   -0.0237507,   -0.0307316,     0.144429, #sector 9
        0.0,   -0.0144483,   -0.0328234,  -0.00553684, #sector 10
        0.0,     0.113188,    -0.176182,    0.0635706, #sector 11
        0.0,    -0.109776,    -0.727705,    -0.756824, #sector 12
        0.0,   -0.0553061,    -0.056162,   -0.0336955, #sector 13
        0.0,    -0.070077,    -0.127203,    -0.140082, #sector 14
        0.0,   -0.0844368,   -0.0735331,    -0.016667, #sector 15
        0.0,    0.0426443,    -0.153659,    -0.129711, #sector 16
    ]

    phiposAlignConsts = [ # 1st, 2nd, 3rd, 4th layer 
        0.0, 0.0, 0.0, 0.0, #-16
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-14
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-12
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-10
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-8
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-6
        0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, #-4
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #-2
        0.0, 0.0, 0.0, 0.0, #-1  
        0.0, 0.0, 0.0, 0.0, # sector =1 and above...
        0.0, 0.0, 0.0, 0.0, #2
        0.0, 0.0, 0.0, 0.0, #3
        0.0, 0.0, 0.0, 0.0, #4
        0.0, 0.0, 0.0, 0.0, 
        0.0, 0.0, 0.0, 0.0, #6
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #8
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #10
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #12
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, #14
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0  #16
    ]
    kwargs.setdefault("phiposAlignConsts", phiposAlignConsts)
    kwargs.setdefault("etaposAlignConsts", etaposAlignConsts)
    kwargs.setdefault("useAlignment", flags.Input.isMC is False and flags.Muon.Align.UseILines is False  )

    return CscAlignmentTool(**kwargs)
    

def QratCscClusterFitterCfg(flags, **kwargs):
    # This is based on https://gitlab.cern.ch/atlas/athena/blob/master/MuonSpectrometer/MuonReconstruction/MuonRecExample/python/CscTools.py#L151
    QratCscClusterFitter=CompFactory.QratCscClusterFitter
    result=ComponentAccumulator()
    qratcor_css_eta = [
        0.000000, 0.000000, 0.000000, 0.000000, 0.0699381, 0.178291, 0.271303, 0.345611, 
        0.400738, 0.452451, 0.493772, 0.526792, 0.553845, 0.580111, 0.603337, 0.624749, 
        0.646065, 0.663924, 0.681897, 0.701345, 0.716067, 0.730324, 0.743168, 0.760598, 
        0.773341, 0.786406, 0.798358, 0.811339, 0.820949, 0.832676, 0.843917, 0.85412, 
        0.863914, 0.873229, 0.88251, 0.892856, 0.903296, 0.910067, 0.917892, 0.928028, 
        0.93461, 0.943679, 0.951617, 0.958326, 0.964428, 0.97341, 0.978204, 0.987075, 
        0.994789, 1.000000
        ]
    qratcor_csl_eta = [
        0.000000, 0.000000, 0.000000, 0.000000, 0.0290177, 0.0834867, 0.188683, 0.269967, 
        0.334887, 0.393036, 0.440317, 0.481884, 0.51725, 0.545107, 0.56959, 0.592327, 
        0.614087, 0.635344, 0.655252, 0.672766, 0.690929, 0.707293, 0.722742, 0.738323, 
        0.753562, 0.76749, 0.780983, 0.79354, 0.806521, 0.818085, 0.829987, 0.841443, 
        0.853723, 0.863393, 0.87431, 0.883139, 0.892962, 0.902803, 0.911276, 0.921417, 
        0.929387, 0.938106, 0.947072, 0.954406, 0.961692, 0.97055, 0.978517, 0.985133, 
        0.992257, 1.000000
        ]
    kwargs.setdefault("qratcor_csl_eta",qratcor_csl_eta)
    kwargs.setdefault("qratcor_css_eta",qratcor_css_eta)
    csc_align_tool = CscAlignmentTool(flags)
    kwargs.setdefault("CscAlignmentTool", csc_align_tool )
    result.setPrivateTools(QratCscClusterFitter(**kwargs))
    
    return result

def CalibCscStripFitterCfg(flags, name = "CalibCscStripFitter",**kwargs):
    CalibCscStripFitter=CompFactory.CalibCscStripFitter
    from MuonConfig.MuonCalibrationConfig import CscCalibToolCfg
    result = CscCalibToolCfg(flags)
    kwargs.setdefault("cscCalibTool", result.popPrivateTools() )
    result.setPrivateTools(CalibCscStripFitter(name=name,**kwargs))
    return result

def CscClusterUtilToolCfg(flags, name='CscClusterUtilTool', **kwargs):
    CscClusterUtilTool=CompFactory.CscClusterUtilTool
    
    result = CalibCscStripFitterCfg(flags)
    kwargs.setdefault("strip_fitter", result.popPrivateTools() )
    
    acc = QratCscClusterFitterCfg(flags)
    kwargs.setdefault("precision_fitter", acc.popPrivateTools() )
    result.setPrivateTools(CscClusterUtilTool(name=name, **kwargs))
    result.merge(acc)
    return result

def CscSegmentUtilToolCfg(flags, name='CscSegmentUtilTool', **kwargs):
    CscSegmentUtilTool=CompFactory.CscSegmentUtilTool
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import CscClusterOnTrackCreatorCfg
    
    result=CscClusterOnTrackCreatorCfg(flags)
    csc_cluster_creator = result.popPrivateTools()
    kwargs.setdefault("rot_creator", csc_cluster_creator )
    
    result.setPrivateTools(CscSegmentUtilTool( name=name, **kwargs))
    return result

def Csc2dSegmentMakerCfg(flags, name= "Csc2dSegmentMaker", **kwargs):
    Csc2dSegmentMaker=CompFactory.Csc2dSegmentMaker
    result=ComponentAccumulator()
    if 'segmentTool' not in kwargs:
        acc  = CscSegmentUtilToolCfg(flags)
        csc_segment_util_tool = acc.getPrimary()
        result.addPublicTool(csc_segment_util_tool)
        kwargs.setdefault('segmentTool', csc_segment_util_tool)
        result.merge(acc)
    
    kwargs.setdefault("printerTool", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags) ))

    csc_segment_maker = Csc2dSegmentMaker(name=name, **kwargs)
    result.setPrivateTools(csc_segment_maker)
    
    return result


def Csc4dSegmentMakerCfg(flags, name= "Csc4dSegmentMaker", **kwargs):    
    result=CscSegmentUtilToolCfg(flags)
    kwargs.setdefault("segmentTool",  result.getPrimary() )
    kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) )
    result.setPrivateTools(CompFactory.Csc4dSegmentMaker(name=name, **kwargs))
    return result

def MuonNSWSegmentFinderToolCfg(flags, name='MuonNSWSegmentFinderTool', **kwargs):

    from MuonConfig.MuonRecToolsConfig import MuonTrackToSegmentToolCfg
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg, MuonTrackSummaryToolCfg
    result=ComponentAccumulator()
    kwargs.setdefault("IPConstraint", flags.Beam.Type is BeamType.Collisions)
    kwargs.setdefault('SegmentAmbiguityTool', result.popToolsAndMerge( MuonAmbiProcessorCfg(flags) ) ) 
    kwargs.setdefault('SLFitter', result.popToolsAndMerge( MCTBSLFitterMaterialFromTrackCfg(flags) ) ) 
    kwargs.setdefault("TrackToSegmentTool", result.popToolsAndMerge( MuonTrackToSegmentToolCfg(flags) ) )
    kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) )
    if flags.Beam.Type is not BeamType.Collisions:
        kwargs.setdefault("MmOccupancyBinWidth", 2048)

    kwargs.setdefault('TrackCleaner', result.popToolsAndMerge( MuonTrackCleanerCfg(flags, name='MuonTrackCleaner_seg',seg=True) ) ) 
    if flags.Muon.MuonTrigger:
        kwargs.setdefault('TrackSummaryTool', result.popToolsAndMerge( MuonTrackSummaryToolCfg(flags, name='MuonTrackSummary') ) ) 
    else:
        kwargs.setdefault('TrackSummaryTool', result.popToolsAndMerge( MuonCombinedTrackSummaryToolCfg(flags, name='CombinedMuonTrackSummary') ) ) 
    
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg
    kwargs.setdefault("MuonClusterCreator", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)) )

    result.setPrivateTools(CompFactory.Muon.MuonNSWSegmentFinderTool(name,**kwargs))
    return result

def MuonPRDSelectionToolCfg( flags, name="MuonPRDSelectionTool", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg, MdtDriftCircleOnTrackCreatorCfg
    # Won't explicitly configure MuonIdHelperSvc
    result = ComponentAccumulator()
    kwargs.setdefault("MdtDriftCircleOnTrackCreator", result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags)))
    kwargs.setdefault("MuonClusterOnTrackCreator",    result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))
    # Won't explicitly configure MuonRecoValidationTool (but it would need configuration if used)
    the_tool = CompFactory.Muon.MuonPRDSelectionTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonClusterSegmentFinderCfg(flags, name = " MuonClusterSegmentFinder", **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonTrackToSegmentToolCfg
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg

    result=ComponentAccumulator()

    # Won't explicitly configure MuonIdHelperSvc
    edm_printer = result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags) )
    kwargs.setdefault("MuonEDMPrinterTool", edm_printer)
    # Won't explicitly configure MuonLayerHashProviderTool
    kwargs.setdefault("MuonPRDSelectionTool", result.popToolsAndMerge( MuonPRDSelectionToolCfg(flags) ) )
    kwargs.setdefault('MdtSegmentMaker', result.popToolsAndMerge( DCMathSegmentMakerCfg(flags,name="DCMathSegmentMaker") ) ) 
    # Won't explicitly configure MuonClusterizationTool
    kwargs.setdefault("MuonClusterOnTrackCreator", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)) )
    kwargs.setdefault("TrackToSegmentTool", result.popToolsAndMerge( MuonTrackToSegmentToolCfg(flags) ) )
    kwargs.setdefault('SLFitter', result.popToolsAndMerge( MCTBSLFitterCfg(flags) ) ) 
    kwargs.setdefault('AmbiguityProcessor', result.popToolsAndMerge( MuonAmbiProcessorCfg(flags) ) ) 
    kwargs.setdefault('TrackCleaner', result.popToolsAndMerge( MuonTrackCleanerCfg(flags) ) ) 
    kwargs.setdefault('MuonSegmentOverlapRemovalTool', CompFactory.Muon.MuonSegmentOverlapRemovalTool(Printer=edm_printer)) 
    
    # Won't explicitly configure MuonSegmentOverlapRemovalTool (though it possibly needs it)

    from MuonConfig.MuonRecToolsConfig import MuonTrackToSegmentToolCfg
    kwargs.setdefault( "TrackToSegmentTool", result.popToolsAndMerge(MuonTrackToSegmentToolCfg(flags)))
    result.setPrivateTools(CompFactory.Muon.MuonClusterSegmentFinder(name, **kwargs))
    return result

def MuonLayerHoughToolCfg(flags, name = "MuonLayerHoughTool" , **kwargs):
    result = ComponentAccumulator()
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("DoTruth", False)
    else:
        kwargs.setdefault("DoTruth", flags.Input.isMC)
    layer_hough_tool = CompFactory.Muon.MuonLayerHoughTool(name, **kwargs)
    result.setPrivateTools(layer_hough_tool)
    return result

def MuonLayerHoughAlgCfg(flags, name = "MuonLayerHoughAlg", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault('TgcPrepDataContainer', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    kwargs.setdefault("CscPrepDataContainer", "CSC_Clusters" if flags.Detector.EnableCSC else "")
    kwargs.setdefault("sTgcPrepDataContainer", "STGC_Measurements" if flags.Detector.EnablesTGC else "")
    kwargs.setdefault('TgcPrepDataContainer', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')
    kwargs.setdefault("MMPrepDataContainer", "MM_Measurements" if flags.Detector.EnableMM else "")
    kwargs.setdefault("PrintSummary", flags.Muon.printSummary)
    kwargs.setdefault("printerTool", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags) ))

    hough_tool = result.getPrimaryAndMerge(MuonLayerHoughToolCfg(flags,
                                            name = "MuonLayerHoughTool")) if flags.Beam.Type is not BeamType.Cosmics else \
                 result.popToolsAndMerge(MuonHoughPatternFinderToolCfg(flags, name="MuonHoughPatternFinderTool"))
    kwargs.setdefault("MuonLayerScanTool", hough_tool)
    
    the_alg = CompFactory.MuonLayerHoughAlg(name = name,**kwargs)
    result.addEventAlgo( the_alg, primary = True )         
    return result


def MuonPatternCalibrationCfg(flags, name="MuonPatternCalibration", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg, MdtDriftCircleOnTrackCreatorCfg, MdtCalibToolForRotsCfg
    result = ComponentAccumulator()
    
    if "MdtCreator" not in kwargs: 
        # on data configure a MdtDriftCircleOnTrackCreator for the segment finding with reduced errors
        # when using the t0 refit enlarge the time window
        if not flags.Input.isMC and flags.Beam.Type is BeamType.Collisions:
            if flags.Muon.doSegmentT0Fit:
                timeWindowSetting = MdtCalibWindowNumber('Collision_t0fit')
            else:
                timeWindowSetting = MdtCalibWindowNumber('Collision_data')
            acc = MdtDriftCircleOnTrackCreatorCfg(flags, name="MdtDriftCircleOnTrackCreatorSegmentFinding", 
                                                         CreateTubeHit = False,
                                                         CalibrationTool = result.popToolsAndMerge(MdtCalibToolForRotsCfg(flags,
                                                                                                        TimeWindowSetting = timeWindowSetting)) 
                                                         )   
        else:
            # I think we need to configure a 'default' version of the MdtDriftCircleOnTrackCreator here
            acc = MdtDriftCircleOnTrackCreatorCfg(flags)

        kwargs.setdefault('MdtCreator', result.popToolsAndMerge(acc))
    
    kwargs.setdefault('ClusterCreator', result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))
    kwargs.setdefault("Printer", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags) ))
    kwargs.setdefault('TgcPrepDataContainer', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("AngleCutPhi", 1e9)
        kwargs.setdefault("DropDistance", 100000000.)
    # Won't explicitly configure MuonIdHelperSvc
    result.setPrivateTools( CompFactory.Muon.MuonPatternCalibration(name, **kwargs) )
    return result

def MuonSegmentFinderNCBAlgCfg(flags, name="MuonSegmentMaker_NCB", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg
    result = ComponentAccumulator()
    ### Only use the TGC measurements from the  current bunch crossing
    kwargs.setdefault("TGC_PRDs", "TGC_Measurements")
    kwargs.setdefault("doStgcSegments", flags.Detector.EnablesTGC)
    kwargs.setdefault("doMMSegments", flags.Detector.EnableMM)
    kwargs.setdefault("doMdtSegments", False)
    kwargs.setdefault("CSC_clusterkey", "CSC_Clusters" if flags.Detector.EnableCSC else "")
    ## Define the output container
    kwargs.setdefault("SegmentCollectionName", "NCB_TrackMuonSegments")
    ### Do not rebuild the NSW alignment segments
    kwargs.setdefault("NSWSegmentCollectionName", "")
    kwargs.setdefault("SegmentQuality", 1)


    ### Setup the CSC segment maker
    if flags.Detector.EnableCSC:
        csc_segment_util_tool = result.popToolsAndMerge(CscSegmentUtilToolCfg(flags, 
                                                                              name='CscSegmentUtilTool_NCB', 
                                                                              TightenChi2 = False, 
                                                                              IPconstraint=False))
      
        csc_2d_segment_maker = result.popToolsAndMerge(Csc2dSegmentMakerCfg(flags, 
                                                                            name='Csc2dSegmentMaker_NCB', 
                                                                            segmentTool=csc_segment_util_tool)) 
        
        csc_4d_segment_maker = result.popToolsAndMerge(Csc4dSegmentMakerCfg(flags, 
                                                                            name='Csc4dSegmentMaker_NCB', 
                                                                            segmentTool=csc_segment_util_tool))

        kwargs.setdefault("Csc2dSegmentMaker", csc_2d_segment_maker)
        kwargs.setdefault("Csc4dSegmentMaker", csc_4d_segment_maker)
    ### Setup the NSW segment maker
    if flags.Detector.EnablesTGC or flags.Detector.EnableMM:
        kwargs.setdefault("MuonClusterCreator", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))
        # Configure subtools needed by MuonNSWSegmentFinderTool
        extrapolator = result.getPrimaryAndMerge(MuonStraightLineExtrapolatorCfg(flags))
        result.addPublicTool(extrapolator)
        cleaner = result.popToolsAndMerge( MuonTrackCleanerCfg(flags, name='MuonTrackCleaner_seg',seg=True) )
        segment_finder = result.popToolsAndMerge(MuonNSWSegmentFinderToolCfg(flags, 
                                                                                   name ="MuonNSWSegmentFinderTool", 
                                                                                   TrackCleaner = cleaner,
                                                                                   SeedMMStereos = False,
                                                                                   IPConstraint = False))
        kwargs.setdefault("NSWSegmentMaker", segment_finder)
    the_alg = CompFactory.MuonSegmentFinderAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True) 
    return result

def MuonSegmentFinderAlgCfg(flags, name="MuonSegmentMaker", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MuonClusterOnTrackCreatorCfg
    result = ComponentAccumulator()
    # Won't configure MuonIdHelperSvc
    kwargs.setdefault("EDMPrinter", result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) )
    
    # for test purposes allow parallel running of truth segment finding and new segment finder
    kwargs.setdefault('MuonPatternCalibration', result.popToolsAndMerge( MuonPatternCalibrationCfg(flags) ) )   
    segment_maker = result.getPrimaryAndMerge(DCMathSegmentMakerCfg(flags,name="DCMathSegmentMaker"))
    kwargs.setdefault('SegmentMaker', segment_maker)
    kwargs.setdefault("MuonClusterSegmentFinder", result.popToolsAndMerge(MuonClusterSegmentFinderCfg(flags, name = "MuonClusterSegmentFinder")))
    # Not yet configuring MuonSegmentOverlapRemovalTool
    
    if flags.Detector.EnableMM or flags.Detector.EnablesTGC:
        kwargs.setdefault("MuonClusterCreator", result.popToolsAndMerge(MuonClusterOnTrackCreatorCfg(flags)))

        # Configure subtools needed by MuonNSWSegmentFinderTool
        extrapolator = result.getPrimaryAndMerge(MuonStraightLineExtrapolatorCfg(flags))
        result.addPublicTool(extrapolator)
        cleaner = result.popToolsAndMerge( MuonTrackCleanerCfg(flags, name='MuonTrackCleaner_seg',seg=True) )
        segment_finder = result.getPrimaryAndMerge(MuonNSWSegmentFinderToolCfg(flags, name ="MuonNSWSegmentFinderTool", TrackCleaner = cleaner))
        kwargs.setdefault("NSWSegmentMaker", segment_finder)

    ### we check whether the layout contains any CSC chamber and if yes, we check that the user also wants to use the CSCs in reconstruction    
    if flags.Detector.EnableCSC:
        kwargs.setdefault("Csc2dSegmentMaker", result.popToolsAndMerge(Csc2dSegmentMakerCfg(flags)))
        kwargs.setdefault("Csc4dSegmentMaker", result.popToolsAndMerge(Csc4dSegmentMakerCfg(flags)))
    else:        
        kwargs.setdefault("CSC_clusterkey", "")
    
    kwargs.setdefault("PrintSummary", flags.Muon.printSummary)
    kwargs.setdefault("doStgcSegments", flags.Detector.EnablesTGC)
    kwargs.setdefault("doMMSegments", flags.Detector.EnableMM)

    kwargs.setdefault("SegmentCollectionName",  "TrackMuonSegments" if flags.Muon.segmentOrigin != "TruthTracking" else "ThirdChainSegments")
    kwargs.setdefault('TGC_PRDs', 'TGC_MeasurementsAllBCs' if not flags.Muon.useTGCPriorNextBC else 'TGC_Measurements')

    the_alg = CompFactory.MuonSegmentFinderAlg( name, **kwargs)                                               
    result.addEventAlgo(the_alg)
    return result    

def MuonSegmentFilterAlgCfg(flags, name="MuonSegmentFilterAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("SegmentCollectionName", "TrackMuonSegments")
    ## The output key of this alg is per default FilteredMuonSegments
    kwargs.setdefault("FilteredCollectionName", "TrackMuonSegmentsEMEO")
    the_alg =  CompFactory.MuonSegmentFilterAlg(name, **kwargs)
    result.addEventAlgo(the_alg)
    return result

def MuonSegmentCnvAlgCfg(flags, name="MuonSegmentCnvAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedRecToolsConfig import MuonSegmentConverterToolCfg
    kwargs.setdefault("MuonSegmentConverterTool", result.popToolsAndMerge(MuonSegmentConverterToolCfg(flags)))
    the_alg = CompFactory.xAODMaker.MuonSegmentCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def MuonSegmentFindingCfg(flags, setup_bytestream = True, cardinality=1):
    """
    Returns a CA setting up Muon Segment Finding
    @param setup_bytestream if True and if Format.BS, sets up reading from bytestream. If False, disables setting up BS, even if Format.BS is True 
    """
    # Set up some general stuff needed by muon reconstruction

    result = ComponentAccumulator()
    
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg 
    result.merge( MuonGeoModelCfg(flags) )
    from MuonConfig.MuonRecToolsConfig import MuonEDMHelperSvcCfg
    result.merge(MuonEDMHelperSvcCfg(flags))    

    if (setup_bytestream):
        # We need to be able to disable this when using MuonCalibStream, 
        # as the normal BS convertors clash with the special MCS converters.
        if flags.Input.Format is Format.BS:
            from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
            result.merge( MuonByteStreamDecodersCfg(flags) )

        if flags.Input.Format is Format.BS or 'StreamRDO' in flags.Input.ProcessingTags:
            from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
            result.merge( MuonRDOtoPRDConvertorsCfg(flags) )
  
    # We need to add two algorithms - one for normal collisions, one for NCB
    result.merge(MuonLayerHoughAlgCfg(flags))
    result.merge(MuonSegmentFinderAlgCfg(flags, name="MuonSegmentMaker"))
  
    
    if flags.Muon.runCommissioningChain:
        result.merge(MuonSegmentFilterAlgCfg(flags))

    if flags.Beam.Type is BeamType.Collisions:
        result.merge(MuonSegmentFinderNCBAlgCfg(flags))
        result.merge(MuonSegmentCnvAlgCfg(flags, "MuonSegmentCnvAlg_NCB",
                                                  SegmentContainerName="NCB_TrackMuonSegments",
                                                  xAODContainerName="NCB_MuonSegments") )

    if flags.Detector.EnableMM or flags.Detector.EnablesTGC:
        result.merge(MuonSegmentCnvAlgCfg(flags, "QuadNSW_MuonSegmentCnvAlg",
                                                 SegmentContainerName="TrackMuonNSWSegments",
                                                 xAODContainerName="xAODNSWSegments"))
    return result

if __name__=="__main__":
    # To run this, do e.g. 
    # python -m MuonConfig.MuonSegmentFindingConfig --threads=1
    from MuonConfig.MuonConfigUtils import SetupMuonStandaloneConfigFlags, SetupMuonStandaloneOutput, SetupMuonStandaloneCA

    args, flags = SetupMuonStandaloneConfigFlags()
    cfg = SetupMuonStandaloneCA(args, flags)

    # Run the actual test.
    acc = MuonSegmentFindingCfg(flags)
    cfg.merge(acc)

    # This is a temporary fix - it should go someplace central as it replaces the functionality of addInputRename from here:
    # https://gitlab.cern.ch/atlas/athena/blob/master/Control/SGComps/python/AddressRemappingSvc.py
    AddressRemappingSvc, ProxyProviderSvc=CompFactory.getComps("AddressRemappingSvc","ProxyProviderSvc",)
    pps = ProxyProviderSvc()
    ars=AddressRemappingSvc()
    pps.ProviderNames += [ 'AddressRemappingSvc' ]
    ars.TypeKeyRenameMaps += [ '%s#%s->%s' % ("Trk::SegmentCollection", "TrackMuonSegments", "TrackMuonSegments_old") ]
    ars.TypeKeyRenameMaps += [ '%s#%s->%s' % ("Trk::SegmentCollection", "NCB_TrackMuonSegments", "NCB_TrackMuonSegments_old") ]

    cfg.addService(pps)
    cfg.addService(ars)

    itemsToRecord = ["Trk::SegmentCollection#TrackMuonSegments", "Trk::SegmentCollection#NCB_TrackMuonSegments"]
    SetupMuonStandaloneOutput(cfg, flags, itemsToRecord)

    # cfg.getService("StoreGateSvc").Dump = True
    cfg.printConfig()
    f=open("MuonSegmentFinding.pkl","wb")
    cfg.store(f)
    f.close()
    
    if not args.config_only:
        sc = cfg.run(20)
        if not sc.isSuccess():
            import sys
            sys.exit("Execution failed")
    else:
        cfg.wasMerged()
