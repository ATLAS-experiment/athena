#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetTrackPerfMonConfig.py
@author M. Aparo
@date 2023-02-17
@brief Main CA-based python configuration for InDetTrackPerfMonTool
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

from InDetTrackPerfMon.ConfigUtils import get_flags
from InDetTrackPerfMon.ConfigUtils import get_opt
from InDetTrackPerfMon.ConfigUtils import has_in
from InDetTrackPerfMon.ConfigUtils import kwargs_setdefault


def JsonPlotsDefReadToolCfg( flags, name="JsonPlotsDefReadTool", **kwargs ):
    '''
    Tool to read the plots definitions from an input file in JSON format
    '''
    log = logging.getLogger( "JsonPlotsDefReadTool" )
    acc = ComponentAccumulator()

    ## Getting list of strings with plots definitions
    from InDetTrackPerfMon.ConfigUtils import getPlotsDefList
    plotsDefList = getPlotsDefList( flags )
    log.debug( "Loading the following plot definitions:" )
    for plotDef in plotsDefList : log.debug( "\t-> %s", plotDef )

    kwargs.setdefault( "PlotsDefs", plotsDefList )

    acc.setPrivateTools(
        CompFactory.IDTPM.JsonPlotsDefReadTool( name, **kwargs ) )
    return acc


def PlotsDefReadToolCfg( flags, name="PlotsDefReadTool", **kwargs ):
    '''
    CA-based configuration for the Tool to read the plots definition
    '''
    log = logging.getLogger( "PlotsDefReadTool" )

    if flags.PhysVal.IDTPM.plotsDefFormat == "JSON" :
        return JsonPlotsDefReadToolCfg(
            flags, name = "JsonPlotsDefReadTool" +
                flags.PhysVal.IDTPM.currentTrkAna.anaTag, **kwargs )

    log.error( "Non supported plots definition file type %s",
               flags.PhysVal.IDTPM.plotsDefFormat )
    return None


def PlotsDefinitionSvcCfg( flags, name="PlotsDefSvc", **kwargs ):
    '''
    CA-based configuration for the PlotsDefinition Service
    '''
    acc = ComponentAccumulator()

    if "PlotsDefReadTool" not in kwargs:
        kwargs.setdefault( "PlotsDefReadTool", acc.popToolsAndMerge(
            PlotsDefReadToolCfg( flags ) ) )

    acc.addService(
        CompFactory.PlotsDefinitionSvc( name, **kwargs ) )
    return acc


def TrackAnalysisInfoWriteToolCfg( flags, name="TrackAnalysisInfoWriteTool", **kwargs ):
    '''
    Tool to write TrackAnalysisInfo to StoreGate
    '''

    iflags, iname = get_flags( flags, name )

    acc = ComponentAccumulator()

    kwargs_setdefault( kwargs, "AnaTag", iflags, "anaTag" )

    acc.setPrivateTools( CompFactory.IDTPM.TrackAnalysisInfoWriteTool( iname, **kwargs ) )
    return acc


def TrackAnalysisDefinitionSvcCfg( flags, name="TrkAnaDefSvc", **kwargs ):
    '''
    CA-based configuration for the TrackAnalysisDefinition Service
    '''

    iflags, iname = get_flags( flags, name )
    
    log = logging.getLogger( "TrkAnaDefSvc"+iname )
    
    acc = ComponentAccumulator()

    kwargs_setdefault( kwargs, "DirName",          iflags, "DirName" )
    kwargs_setdefault( kwargs, "sortPlotsByChain", iflags, "sortPlotsByChain" )
    kwargs_setdefault( kwargs, "SubFolder",        iflags, "SubFolder" )
    kwargs_setdefault( kwargs, "TrkAnaTag",        iflags, "anaTag" )

    kwargs_setdefault( kwargs, "TestType", iflags, "TestType" )
    kwargs_setdefault( kwargs, "RefType",  iflags, "RefType" )
    kwargs_setdefault( kwargs, "doTrigNavigation",  iflags, "doTrigNavigation" )

    kwargs_setdefault( kwargs, "pileupSwitch",  iflags, "pileupSwitch" )
    # what abomination is this ???
    kwargs.setdefault( "hasFullPileupTruth",
                        ( "xAOD::TruthPileupEventContainer#TruthPileupEvents" in flags.Input.TypedCollections ) )

    from InDetTrackPerfMon.ConfigUtils import getTag

    kwargs_setdefault( kwargs, "TestTag", getTag( flags, get_opt( iflags, "TestType" ) ) )
    kwargs_setdefault( kwargs, "RefTag",  getTag( flags, iflags.RefType ) )

    kwargs_setdefault( kwargs, "MatchingType",      iflags, "MatchingType" )
    kwargs_setdefault( kwargs, "MatchingTruthProb", iflags, "truthProbCut" )

    kwargs_setdefault( kwargs, "ChainNames", iflags, "ChainNames" )
    if ( get_opt( iflags, "doTrigNavigation", False ) and not get_opt( iflags, "ChainNames", [] ) ):
        log.error( "Trying to set up Trigger navigation without specifying any trigger chain" )
        return None

    kwargs_setdefault( kwargs, "plotTrackParameters",       iflags, "plotTrackParameters" )
    kwargs_setdefault( kwargs, "plotTrackParametersErrors", iflags, "plotTrackParametersErrors" )
    kwargs_setdefault( kwargs, "plotTrackMultiplicities",   iflags, "plotTrackMultiplicities" )
    kwargs_setdefault( kwargs, "plotEfficiencies",          iflags, "plotEfficiencies" )
    kwargs_setdefault( kwargs, "plotTechnicalEfficiencies", iflags, "plotTechnicalEfficiencies" )
    kwargs_setdefault( kwargs, "plotResolutions",           iflags, "plotResolutions" )
    kwargs_setdefault( kwargs, "plotFakeRates",             iflags, "plotFakeRates" )
    kwargs_setdefault( kwargs, "unlinkedAsFakes",           iflags, "unlinkedAsFakes" )
    kwargs_setdefault( kwargs, "plotDuplicateRates",        iflags, "plotDuplicateRates" )
    kwargs_setdefault( kwargs, "plotHitsOnTracks",          iflags, "plotHitsOnTracks" )
    kwargs_setdefault( kwargs, "plotHitsOnTracksExpert",    iflags, "plotHitsOnTracksExpert" )
    kwargs_setdefault( kwargs, "plotHitsOnTracksReference", iflags, "plotHitsOnTracksReference" )
    kwargs_setdefault( kwargs, "plotHitsOnMatchedTracks",   iflags, "plotHitsOnMatchedTracks" )
    kwargs_setdefault( kwargs, "plotHitsOnFakeTracks",      iflags, "plotHitsOnFakeTracks" )
    kwargs_setdefault( kwargs, "plotVertexParameters",      iflags, "plotVertexParameters" )
    kwargs_setdefault( kwargs, "useSelectedVertexTracks",   iflags, "useSelectedVertexTracks" )
    kwargs_setdefault( kwargs, "plotOfflineElectrons",      iflags, "plotOfflineElectrons" )
    kwargs_setdefault( kwargs, "ResolutionMethod",          iflags, "ResolutionMethod" )
    kwargs.setdefault( "isITk", flags.Detector.GeometryITk )
    #  AAAAAAARGHHHH !!!!!!!!!!!
    if has_in( "Jet", iflags, "SelectOfflineObject" ):
        kwargs.setdefault( "plotTracksInJets", True )
    else:
        kwargs.setdefault( "plotTracksInJets", False )

    if flags.Detector.GeometryITk: 
         kwargs_setdefault( kwargs, "EtaBins",    iflags, "etaBins", [] )
         kwargs_setdefault( kwargs, "MinSilHits", iflags, "minClusters" )
    else:
         kwargs.setdefault( "EtaBins", [-1, 9999.] ) # for technical efficiencies ?????
         kwargs.setdefault( "MinSilHits", [flags.Tracking.MainPass.minClusters] ) # for technical efficiencies ????

    trkAnaDefSvc = CompFactory.TrackAnalysisDefinitionSvc( name, **kwargs )
    acc.addService( trkAnaDefSvc )
    return acc


def InDetTrackPerfMonToolCfg( flags, name="InDetTrackPerfMonTool", **kwargs ):
    '''
    Main IDTPM tool instance CA-based configuration
    '''
    log = logging.getLogger( "InDetTrackPerfMonTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    acc = ComponentAccumulator()

    kwargs.setdefault( "AnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    ## Track and Vertex collections
    kwargs.setdefault( "OfflineTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.OfflineTrkKey )
    kwargs.setdefault( "OfflineVertexContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.OfflineVtxKey )

    kwargs.setdefault( "TruthParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TruthPartKey )
    kwargs.setdefault( "TruthVertexContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TruthVtxKey )

    kwargs.setdefault( "TriggerTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )
    kwargs.setdefault( "TriggerVertexContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TrigVtxKey )

    ## TrackAnalysisInfoWriteTool
    if flags.Output.doWriteAOD_IDTPM :
        kwargs.setdefault( "writeOut", True )
        kwargs.setdefault( "TrkAnaInfoKey",
                           "TrkAnaInfo"+flags.PhysVal.IDTPM.currentTrkAna.anaTag )

        if "TrackAnalysisInfoWriteTool" not in kwargs :
            kwargs.setdefault( "TrackAnalysisInfoWriteTool", acc.popToolsAndMerge(
                TrackAnalysisInfoWriteToolCfg( flags,
                    name="TrackAnalysisInfoWriteTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    ## TrackAnalysisDefinitionSvc
    acc.merge( TrackAnalysisDefinitionSvcCfg( flags,
                   name="TrkAnaDefSvc"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) )

    ## PlotsDefinitionSvc
    acc.merge( PlotsDefinitionSvcCfg( flags,
                    name="PlotsDefSvc"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) )

    ## Decorator algorithms
    ## Truth-Hit decorator
    if ( ( "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType ) or
         ( "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType ) ):
        if not flags.Input.isMC:
            log.error( "Trying to use Truth collections with non-MC sample." )
            return None

        from InDetTrackPerfMon.InDetAlgorithmConfig import TruthHitDecoratorAlgCfg, TruthDecoratorAlgCfg
        acc.merge( TruthHitDecoratorAlgCfg( flags ) )
        # FIXME This algorithm should not be scheduled if the decorations already exist.
        acc.merge( TruthDecoratorAlgCfg( flags ) )

    ## Offline track-object decorator
    if ( ( flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject ) and
         ( "Truth" not in flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject ) ):
        from InDetTrackPerfMon.InDetAlgorithmConfig import OfflineObjectDecoratorAlgCfg
        acc.merge( OfflineObjectDecoratorAlgCfg( flags ) )

    ## now the sub-tools    
    if "TrackQualitySelectionTool" not in kwargs:
        from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
        kwargs.setdefault( "TrackQualitySelectionTool", acc.popToolsAndMerge(
            TrackQualitySelectionToolCfg( flags,
                name="TrackQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if "VertexQualitySelectionTool" not in kwargs:
        from InDetTrackPerfMon.InDetSelectionConfig import VertexQualitySelectionToolCfg
        kwargs.setdefault( "VertexQualitySelectionTool", acc.popToolsAndMerge(
            VertexQualitySelectionToolCfg( flags,
                name="VertexQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if flags.PhysVal.IDTPM.currentTrkAna.doTrigNavigation:

        if "TrigDecisionTool" not in kwargs:
            from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
            kwargs.setdefault( "TrigDecisionTool",
                               acc.getPrimaryAndMerge( TrigDecisionToolCfg( flags ) ) )

        if "RoiSelectionTool" not in kwargs:
            from InDetTrackPerfMon.InDetSelectionConfig import RoiSelectionToolCfg
            kwargs.setdefault( "RoiSelectionTool", acc.popToolsAndMerge(
                RoiSelectionToolCfg( flags,
                    name="RoiSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

        if "TrackRoiSelectionTool" not in kwargs:
            from InDetTrackPerfMon.InDetSelectionConfig import TrackRoiSelectionToolCfg
            kwargs.setdefault( "TrackRoiSelectionTool", acc.popToolsAndMerge(
                TrackRoiSelectionToolCfg( flags,
                    name="TrackRoiSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

        if "VertexRoiSelectionTool" not in kwargs:
            from InDetTrackPerfMon.InDetSelectionConfig import VertexRoiSelectionToolCfg
            kwargs.setdefault( "VertexRoiSelectionTool", acc.popToolsAndMerge(
                VertexRoiSelectionToolCfg( flags,
                    name="VertexRoiSelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if "TrackMatchingTool" not in kwargs:
        from InDetTrackPerfMon.InDetMatchingConfig import TrackMatchingToolCfg
        matchToolCfg = TrackMatchingToolCfg( flags )
        if matchToolCfg is not None :
            kwargs.setdefault( "doMatch", True ) # = False by default
            kwargs.setdefault( "TrackMatchingTool", acc.popToolsAndMerge( matchToolCfg ) )

    acc.setPrivateTools( CompFactory.InDetTrackPerfMonTool( name, **kwargs ) )
    return acc


def InDetClusterPerfMonToolCfg( flags, name="InDetClusterPerfMonTool", **kwargs ):
    '''
    Tool instance CA-based configuration for cluster and space-points validation
    Currently only available for offline-type analyses (i.e. full-scan) and Run4/ITk/ACTS
    '''

    ## Skipping Trigger Navigation trackAnalysis
    if ( flags.PhysVal.IDTPM.currentTrkAna.RefType == "Trigger" or
         flags.PhysVal.IDTPM.currentTrkAna.TestType == "Trigger" or
         flags.PhysVal.IDTPM.currentTrkAna.doTrigNavigation ):
        return None

    acc = ComponentAccumulator()

    ## Computing output directory string
    folderStr = flags.PhysVal.IDTPM.DirName + "/"
    if flags.PhysVal.IDTPM.sortPlotsByChain :
        folderStr += "Offline/" + flags.PhysVal.IDTPM.currentTrkAna.SubFolder
    else :
        folderStr += flags.PhysVal.IDTPM.currentTrkAna.SubFolder + "/Offline"
    kwargs.setdefault( "folder", folderStr )

    typedCollections = flags.Input.TypedCollections

    ## ITk pixel and strip cluster collections
    if ( flags.PhysVal.IDTPM.currentTrkAna.PixelClusterKey and
         "xAOD::PixelClusterContainer#"+flags.PhysVal.IDTPM.currentTrkAna.PixelClusterKey in typedCollections ):
        kwargs.setdefault( "doPixelClusters", True )
        kwargs.setdefault( "pixelClustersDirectory", "PixelClusters" )
        kwargs.setdefault( "PixelClusterContainerKey",
                           flags.PhysVal.IDTPM.currentTrkAna.PixelClusterKey )

    if ( flags.PhysVal.IDTPM.currentTrkAna.StripClusterKey and
         "xAOD::StripClusterContainer#"+flags.PhysVal.IDTPM.currentTrkAna.StripClusterKey in typedCollections ):
        kwargs.setdefault( "doStripClusters", True )
        kwargs.setdefault( "stripClustersDirectory", "StripClusters" )
        kwargs.setdefault( "StripClusterContainerKey",
                           flags.PhysVal.IDTPM.currentTrkAna.StripClusterKey )

    ## ITk pixel and strip space points collections
    if ( flags.PhysVal.IDTPM.currentTrkAna.PixelSpacePointKey and
         "xAOD::SpacePointContainer#"+flags.PhysVal.IDTPM.currentTrkAna.PixelSpacePointKey in typedCollections ):
        kwargs.setdefault( "doPixelSpacePoints", True )
        kwargs.setdefault( "pixelSpacePointsDirectory", "PixelSpacePoints" )
        kwargs.setdefault( "PixelSpacePointContainerKey",
                           flags.PhysVal.IDTPM.currentTrkAna.PixelSpacePointKey )

    if ( flags.PhysVal.IDTPM.currentTrkAna.StripSpacePointKey and
         "xAOD::SpacePointContainer#"+flags.PhysVal.IDTPM.currentTrkAna.StripSpacePointKey in typedCollections ):
        kwargs.setdefault( "doStripSpacePoints", True )
        kwargs.setdefault( "stripSpacePointsDirectory", "StripSpacePoints" )
        kwargs.setdefault( "StripSpacePointContainerKey",
                           flags.PhysVal.IDTPM.currentTrkAna.StripSpacePointKey )

    if ( flags.PhysVal.IDTPM.currentTrkAna.StripOverlapSpacePointKey and
         "xAOD::SpacePointContainer#"+flags.PhysVal.IDTPM.currentTrkAna.StripOverlapSpacePointKey in typedCollections ):
        kwargs.setdefault( "doStripOverlapSpacePoints", True )
        kwargs.setdefault( "stripSpaceOverlapPointsDirectory", "StripOverlapSpacePoints" )
        kwargs.setdefault( "StripOverlapSpacePointContainerKey",
                           flags.PhysVal.IDTPM.currentTrkAna.StripOverlapSpacePointKey )

    acc.setPrivateTools( CompFactory.ActsTrk.PhysValTool( name, **kwargs ) )
    return acc


def InDetTrackPerfMonCfg( flags ):
    '''
    CA-based configuration of all tool instances (= TrackAnalyses)
    '''
    log = logging.getLogger( "InDetTrackPerfMonCfg" )
    acc = ComponentAccumulator()

    ## IDTPM tool instances
    tools = []

    print( "trkAnaNames: ", flags.PhysVal.IDTPM.trkAnaNames )

    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames :

        ## cloning flags of current TrackAnalysis to PhysVal.IDTPM.currentTrkAna
        flags_thisTrkAna = flags.cloneAndReplace( "PhysVal.IDTPM.currentTrkAna",
                                                  "PhysVal.IDTPM."+trkAnaName )

        if flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.enabled:
            log.debug( "Scheduling TrackAnalysis: %s",
                       flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.anaTag )

            ## Track performance validation
            tools.append(
                acc.popToolsAndMerge( InDetTrackPerfMonToolCfg( flags_thisTrkAna,
                    name="InDetTrackPerfMonTool"+
                         flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

            ## Cluster and Space Point perfromace validation
            if flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.doClusterValidation :
                tool = InDetClusterPerfMonToolCfg( flags_thisTrkAna,
                            name="InDetClusterPerfMonTool"+
                                 flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.anaTag )
                if tool : tools.append( acc.popToolsAndMerge( tool ) )

    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    acc.merge( PhysValMonitoringCfg( flags, tools=tools ) )

    ## Adding additional output stream for reprocessing file
    if flags.Output.doWriteAOD_IDTPM :
        from InDetTrackPerfMon.InDetOutputConfig import InDetOutputCfg
        acc.merge( InDetOutputCfg(flags) )

    return acc
