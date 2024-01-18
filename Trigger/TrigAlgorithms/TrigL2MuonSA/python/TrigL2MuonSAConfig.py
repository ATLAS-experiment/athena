#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#  This file configs the L2MuonSA reco alg in the newJO way, 
#      but now is located here temporarily until newJO migrations are done in all trigger signatures.
#  This should be moved at somewhere in offline.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


# Get Rpc data decoder for MuFast data preparator
def RpcDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Rpc data preparator for MuFast data preparator
    from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg
    from RegionSelector.RegSelToolConfig import regSelTool_RPC_Cfg

    recRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags, useRun3Config=flags.Trigger.enableL1MuonPhase1))
    RpcDataPreparator = CompFactory.getComp("TrigL2MuonSA::RpcDataPreparator")(
        TrigT1RPCRecRoiTool = recRoiTool,
        RpcClusterPreparator = CompFactory.getComp("TrigL2MuonSA::RpcClusterPreparator")(
            TrigT1RPCRecRoiTool = recRoiTool),
        RegSel_RPC = acc.popToolsAndMerge( regSelTool_RPC_Cfg( flags ) )
    )

    return acc, RpcDataPreparator

# Get Tgc data decoder for MuFast data preparator 
def TgcDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Tgc data preparator for MuFast data preparator
    TgcDataPreparator = CompFactory.getComp("TrigL2MuonSA::TgcDataPreparator")()
 
    return acc, TgcDataPreparator

# Get Mdt data decoder for MuFast data preparator 
def MdtDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Mdt data preparator for MuFast data preparator
    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    MdtDataPreparator = CompFactory.getComp("TrigL2MuonSA::MdtDataPreparator")(
        RegSel_MDT = acc.popToolsAndMerge( regSelTool_MDT_Cfg( flags ) )
    )
    return acc, MdtDataPreparator

# Get Csc data decoder for MuFast data preparator 
def CscDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Csc data preparator for MuFast data preparator
    CscDataPreparator = CompFactory.getComp("TrigL2MuonSA::CscDataPreparator")()

    return acc, CscDataPreparator

def StgcDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Stgc data preparator for MuFast data preparator
    from RegionSelector.RegSelToolConfig import regSelTool_STGC_Cfg
    StgcDataPreparator = CompFactory.getComp("TrigL2MuonSA::StgcDataPreparator")(
        RegSel_STGC = acc.popToolsAndMerge( regSelTool_STGC_Cfg( flags ) )
    )
    return acc, StgcDataPreparator

def MmDataPreparatorCfg( flags ):

    acc = ComponentAccumulator()

    # Set Mm data preparator for MuFast data preparator
    TrigL2MuonSA__MmDataPreparator=CompFactory.getComp("TrigL2MuonSA::MmDataPreparator")
    MmDataPreparator = TrigL2MuonSA__MmDataPreparator()
    from RegionSelector.RegSelToolConfig import regSelTool_MM_Cfg
    MmDataPreparator.RegSel_MM = acc.popToolsAndMerge( regSelTool_MM_Cfg( flags ) )

    return acc, MmDataPreparator

def RpcRoadDefinerCfg( flags ):

    acc = ComponentAccumulator()

    # Set RPC road definer for MuFast data preparator
    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    RpcRoadDefiner = CompFactory.getComp("TrigL2MuonSA::RpcRoadDefiner")(
        RegionSelectionTool = acc.popToolsAndMerge( regSelTool_MDT_Cfg( flags ) )
    )
    return acc, RpcRoadDefiner

def TgcRoadDefinerCfg( flags ):

    acc = ComponentAccumulator()

    # Set TGC road definer for MuFast data preparator
    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    TgcRoadDefiner = CompFactory.getComp("TrigL2MuonSA::TgcRoadDefiner")(
        RegionSelectionTool = acc.popToolsAndMerge( regSelTool_MDT_Cfg( flags ) )
    )
    return acc, TgcRoadDefiner

def ClusterRoadDefinerCfg(flags):

    acc = ComponentAccumulator()

    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    ClusterRoadDefiner = CompFactory.getComp("TrigL2MuonSA::ClusterRoadDefiner")(
        RegionSelectionTool = acc.popToolsAndMerge(regSelTool_MDT_Cfg(flags))
    )
    return acc, ClusterRoadDefiner


def muFastSteeringCfg( flags, roisKey="", setup="", **kwargs ):
    from MuonConfig.MuonCalibrationConfig import MdtCalibrationToolCfg

    acc = ComponentAccumulator()

    # Get RPC decoder
    rpcAcc, RpcDataPreparator = RpcDataPreparatorCfg( flags )
    acc.merge( rpcAcc )

    # Get TGC decoder
    tgcAcc, TgcDataPreparator = TgcDataPreparatorCfg( flags )
    acc.merge( tgcAcc )

    # Get MDT decoder
    mdtAcc, MdtDataPreparator = MdtDataPreparatorCfg( flags  )
    acc.merge( mdtAcc )

    # Get CSC decoder
    if flags.Detector.GeometryCSC:
        cscAcc, CscDataPreparator = CscDataPreparatorCfg( flags )
        acc.merge( cscAcc )
    else:
        CscDataPreparator = ""

    # Get sTGC decoder
    if flags.Detector.GeometrysTGC:
        stgcAcc, StgcDataPreparator = StgcDataPreparatorCfg( flags )
        acc.merge( stgcAcc )
    else:
        StgcDataPreparator = ""

    # Get MM decoder
    if flags.Detector.GeometryMM:
        mmAcc, MmDataPreparator = MmDataPreparatorCfg( flags )
        acc.merge( mmAcc )
    else:
        MmDataPreparator = ""

    # Get RPC road definer
    rpcRDAcc, RpcRoadDefiner = RpcRoadDefinerCfg( flags )
    acc.merge( rpcRDAcc )

    # Get TGC road definer
    tgcRDAcc, TgcRoadDefiner = TgcRoadDefinerCfg( flags )
    acc.merge( tgcRDAcc )

    # Get Cluster Road Definer
    clusRDAcc, ClusterRoadDefiner = ClusterRoadDefinerCfg(flags)
    acc.merge(clusRDAcc)

    # Set MuFast data preparator
    TrigL2MuonSA__MuFastDataPreparator=CompFactory.getComp("TrigL2MuonSA::MuFastDataPreparator")

    from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg
    MuFastDataPreparator = TrigL2MuonSA__MuFastDataPreparator( CSCDataPreparator = CscDataPreparator,
                                                               MDTDataPreparator = MdtDataPreparator,
                                                               RPCDataPreparator = RpcDataPreparator,
                                                               TGCDataPreparator = TgcDataPreparator,
                                                               STGCDataPreparator = StgcDataPreparator,
                                                               MMDataPreparator = MmDataPreparator,
                                                               RpcRoadDefiner = RpcRoadDefiner,
                                                               TgcRoadDefiner = TgcRoadDefiner,
                                                               ClusterRoadDefiner = ClusterRoadDefiner,
                                                               TrigT1RPCRecRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags, useRun3Config=flags.Trigger.enableL1MuonPhase1)) )

    # Setup the station fitter
    TrigL2MuonSA__MuFastStationFitter,TrigL2MuonSA__PtFromAlphaBeta=CompFactory.getComps("TrigL2MuonSA::MuFastStationFitter","TrigL2MuonSA::PtFromAlphaBeta")
    PtFromAlphaBeta = TrigL2MuonSA__PtFromAlphaBeta()
    PtFromAlphaBeta.useCscPt = True
    PtFromAlphaBeta.AvoidMisalignedCSCs = False

    MuFastStationFitter = TrigL2MuonSA__MuFastStationFitter( PtFromAlphaBeta = PtFromAlphaBeta )
    TrigL2MuonSA__MuFastPatternFinder,TrigL2MuonSA__MuFastTrackFitter,TrigL2MuonSA__MuFastTrackExtrapolator,TrigL2MuonSA__MuCalStreamerTool,TrigL2MuonSA__CscSegmentMaker=CompFactory.getComps("TrigL2MuonSA::MuFastPatternFinder","TrigL2MuonSA::MuFastTrackFitter","TrigL2MuonSA::MuFastTrackExtrapolator","TrigL2MuonSA::MuCalStreamerTool","TrigL2MuonSA::CscSegmentMaker")
    MuFastPatternFinder     = TrigL2MuonSA__MuFastPatternFinder(CalibrationTool=acc.popToolsAndMerge( MdtCalibrationToolCfg(flags)))
    MuFastTrackFitter       = TrigL2MuonSA__MuFastTrackFitter()
    MuFastTrackExtrapolator = TrigL2MuonSA__MuFastTrackExtrapolator()

    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg, regSelTool_TGC_Cfg
    MuCalStreamerTool       = TrigL2MuonSA__MuCalStreamerTool(
        RegSel_MDT = acc.popToolsAndMerge(regSelTool_MDT_Cfg(flags)),
        RegSel_TGC = acc.popToolsAndMerge(regSelTool_TGC_Cfg(flags)) )


    CscSegmentMaker         = TrigL2MuonSA__CscSegmentMaker()

    if not flags.Detector.GeometrysTGC and not flags.Detector.GeometryMM:
        MuFastStationFitter.NswStationFitter=""

    # Set Reco alg of muFast step
    useNSW = (( not flags.Muon.runCommissioningChain ) and ( not flags.Muon.disableNSWForL2SA ))
    from TrigL2MuonSA.TrigL2MuonSAMonitoring import TrigL2MuonSAMonitoring
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    muFastAlg = CompFactory.MuFastSteering(
        name                   = "MuFastSteering_Muon"+setup,
        DataPreparator         = MuFastDataPreparator,
        StationFitter          = MuFastStationFitter,
        PatternFinder          = MuFastPatternFinder,
        TrackFitter            = MuFastTrackFitter,
        TrackExtrapolator      = MuFastTrackExtrapolator,
        FtfRoadDefiner         = CompFactory.TrigL2MuonSA.FtfRoadDefiner(
            IOExtrapolator=acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))),
        CalibrationStreamer    = MuCalStreamerTool,
        MuonCalibrationStream = "MuonCalibrationStream"+setup,
        CscSegmentMaker        = CscSegmentMaker,
        MuRoIs                 = roisKey,
        R_WIDTH_TGC_FAILED     = 200,
        R_WIDTH_RPC_FAILED     = 400,
        DoCalibrationStream    = False,
        USE_ROIBASEDACCESS_CSC = True,
        # NSW on/off
        USE_STGC               = useNSW,
        USE_MM                 = useNSW,
        #################################
        # Temporally RoI based data access is disabled for NSW
        USE_ROIBASEDACCESS_STGC= False,
        USE_ROIBASEDACCESS_MM  = False,
        #################################
        RpcErrToDebugStream    = True,
        topoRoad               = True,
        dEtasurrRoI            = 0.14,
        dPhisurrRoI            = 0.14,
        MonTool                = TrigL2MuonSAMonitoring(flags),
        UseRun3Config          = flags.Trigger.enableL1MuonPhase1,
        UseEndcapInnerFromBarrel = True,
        **kwargs )

    # Default backextrapolator is for MC Misaligned Detector
    TrigMuonBackExtrapolator=CompFactory.TrigMuonBackExtrapolator
    muFastAlg.BackExtrapolator = TrigMuonBackExtrapolator( name        = "MisalignedBackExtrapolator",
                                                           Aligned     = False,
                                                           DataSet     = False )

    if setup == '900GeV':
        muFastAlg.WinPt = 4.0
        muFastAlg.Scale_Road_BarrelInner  = 3
        muFastAlg.Scale_Road_BarrelMiddle = 3
        muFastAlg.Scale_Road_BarrelOuter  = 3
    else:
        muFastAlg.WinPt = 6.0
        muFastAlg.Scale_Road_BarrelInner  = 1
        muFastAlg.Scale_Road_BarrelMiddle = 1
        muFastAlg.Scale_Road_BarrelOuter  = 1

    if setup == 'Calib':
        muFastAlg.DoCalibrationStream = True
        muFastAlg.MuonCalDataScouting = False
        muFastAlg.MuonCalBufferSize   = 1024*1024

    elif setup == 'MuonCalibDataScouting':
        muFastAlg.DoCalibrationStream = True
        muFastAlg.MuonCalDataScouting = True
        muFastAlg.MuonCalBufferSize   = 1024*1024

    elif setup == 'l2mtmode':
        muFastAlg.multitrackMode = True
        muFastAlg.doEndcapForl2mt = False

    elif setup == 'IOmode':
        # Do not run topo road and inside-out mode at the same time
        muFastAlg.topoRoad = False
        muFastAlg.InsideOutMode = True

    return acc, muFastAlg

def PtBarrelLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    ptBarrelLUTSvc = CompFactory.getComp("TrigL2MuonSA::PtBarrelLUTSvc")(name = 'PtBarrelLUTSvc')
    ptBarrelLUTSvc.LUTfile = "pt_barrel.lut"
    ptBarrelLUTSvc.SP_LUTfile = "pt_barrelSP_new.lut"
    
    acc.addService( ptBarrelLUTSvc )

    return acc, ptBarrelLUTSvc

def PtBarrelLUTSvcCfg_MC( flags ):

    acc = ComponentAccumulator()
    ptBarrelLUTSvc_MC = CompFactory.getComp("TrigL2MuonSA::PtBarrelLUTSvc")(name = 'PtBarrelLUTSvc_MC')
    ptBarrelLUTSvc_MC.LUTfile = "pt_barrel.mc10.lut"
    acc.addService( ptBarrelLUTSvc_MC )

    return acc, ptBarrelLUTSvc_MC

def PtEndcapLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    ptEndcapLUTSvc = CompFactory.getComp("TrigL2MuonSA::PtEndcapLUTSvc")(name = 'PtEndcapLUTSvc')
    ptEndcapLUTSvc.FileName = "pt_endcap.lut"
    ptEndcapLUTSvc.EMeanLUT = "pt_comb_mean.lut"
    ptEndcapLUTSvc.ESigmaLUT = "pt_comb_sigma.lut"
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        ptEndcapLUTSvc.UseRun3LUT = True
    else:
        ptEndcapLUTSvc.UseRun3LUT = False
    acc.addService( ptEndcapLUTSvc )

    return acc, ptEndcapLUTSvc

def PtEndcapLUTSvcCfg_MC( flags ):

    acc = ComponentAccumulator()
    ptEndcapLUTSvc_MC = CompFactory.getComp("TrigL2MuonSA::PtEndcapLUTSvc")(name = 'PtEndcapLUTSvc_MC')
    ptEndcapLUTSvc_MC.FileName = "pt_endcap.mc10.lut"
    ptEndcapLUTSvc_MC.EMeanLUT = "pt_comb_mean.lut"
    ptEndcapLUTSvc_MC.ESigmaLUT = "pt_comb_sigma.lut"
    if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
        ptEndcapLUTSvc_MC.UseRun3LUT = True
    else:
        ptEndcapLUTSvc_MC.UseRun3LUT = False
    acc.addService( ptEndcapLUTSvc_MC )

    return acc, ptEndcapLUTSvc_MC


def AlignmentBarrelLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    alignmentBarrelLUTSvc = CompFactory.getComp("TrigL2MuonSA::AlignmentBarrelLUTSvc")(name = 'AlignmentBarrelLUTSvc')
    alignmentBarrelLUTSvc.LUTfile = "dZ_barrel.lut"
    acc.addService( alignmentBarrelLUTSvc )

    return acc, alignmentBarrelLUTSvc


@AccumulatorCache
def l2MuFastAlgCfg( flags, roisKey, setup="", **kwargs ):

    acc = ComponentAccumulator()

    if not roisKey:
        from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
        roisKey = mapThresholdToL1RoICollection("MU")

    # Get Reco alg of muFast step
    muFastAcc, muFastFex = muFastSteeringCfg( flags, roisKey, setup, **kwargs )
    acc.merge( muFastAcc )

    # Get services of the Reco alg
    acc.merge( PtBarrelLUTSvcCfg(flags)[0] )   
    acc.merge( PtBarrelLUTSvcCfg_MC(flags)[0] )   
    acc.merge( PtEndcapLUTSvcCfg(flags)[0] )   
    acc.merge( PtEndcapLUTSvcCfg_MC(flags)[0] )   
    acc.merge( AlignmentBarrelLUTSvcCfg(flags)[0] )
    acc.addEventAlgo(muFastFex)

    return acc



if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    cfg = l2MuFastAlgCfg(flags, roisKey="MURoIs")
    cfg.printConfig(withDetails=True, summariseProps=True)
    cfg.wasMerged()
