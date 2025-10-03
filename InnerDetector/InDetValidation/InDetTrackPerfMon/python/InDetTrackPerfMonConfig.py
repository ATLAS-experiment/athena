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
    acc = ComponentAccumulator()

    kwargs.setdefault( "AnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    acc.setPrivateTools( CompFactory.IDTPM.TrackAnalysisInfoWriteTool( name, **kwargs ) )
    return acc


def TrackAnalysisDefinitionSvcCfg( flags, name="TrkAnaDefSvc", **kwargs ):
    '''
    CA-based configuration for the TrackAnalysisDefinition Service
    '''
    log = logging.getLogger( "TrkAnaDefSvc"+flags.PhysVal.IDTPM.currentTrkAna.anaTag )
    acc = ComponentAccumulator()

    kwargs.setdefault( "DirName", flags.PhysVal.IDTPM.DirName )
    kwargs.setdefault( "sortPlotsByChain", flags.PhysVal.IDTPM.sortPlotsByChain )
    kwargs.setdefault( "SubFolder", flags.PhysVal.IDTPM.currentTrkAna.SubFolder )
    kwargs.setdefault( "TrkAnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    kwargs.setdefault( "TestType", flags.PhysVal.IDTPM.currentTrkAna.TestType )
    kwargs.setdefault( "RefType",  flags.PhysVal.IDTPM.currentTrkAna.RefType )
    kwargs.setdefault( "doTrigNavigation",  flags.PhysVal.IDTPM.currentTrkAna.doTrigNavigation )

    kwargs.setdefault( "pileupSwitch",  flags.PhysVal.IDTPM.currentTrkAna.pileupSwitch )
    kwargs.setdefault( "hasFullPileupTruth",
                        ( "xAOD::TruthPileupEventContainer#TruthPileupEvents" in flags.Input.TypedCollections ) )

    from InDetTrackPerfMon.ConfigUtils import getTag
    kwargs.setdefault( "TestTag", getTag( flags, flags.PhysVal.IDTPM.currentTrkAna.TestType ) )
    kwargs.setdefault( "RefTag",  getTag( flags, flags.PhysVal.IDTPM.currentTrkAna.RefType ) )

    kwargs.setdefault( "MatchingType", flags.PhysVal.IDTPM.currentTrkAna.MatchingType )
    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    kwargs.setdefault( "ChainNames", flags.PhysVal.IDTPM.currentTrkAna.ChainNames )
    if ( flags.PhysVal.IDTPM.currentTrkAna.doTrigNavigation and
         not flags.PhysVal.IDTPM.currentTrkAna.ChainNames ):
        log.error( "Trying to set up Trigger navigation without specifying any trigger chain" )
        return None

    kwargs.setdefault( "plotTrackParameters", flags.PhysVal.IDTPM.currentTrkAna.plotTrackParameters )
    kwargs.setdefault( "plotTrackParametersErrors", flags.PhysVal.IDTPM.currentTrkAna.plotTrackParametersErrors )
    kwargs.setdefault( "plotTrackMultiplicities", flags.PhysVal.IDTPM.currentTrkAna.plotTrackMultiplicities )
    kwargs.setdefault( "plotEfficiencies", flags.PhysVal.IDTPM.currentTrkAna.plotEfficiencies )
    kwargs.setdefault( "plotTechnicalEfficiencies", flags.PhysVal.IDTPM.currentTrkAna.plotTechnicalEfficiencies )
    kwargs.setdefault( "plotResolutions", flags.PhysVal.IDTPM.currentTrkAna.plotResolutions )
    kwargs.setdefault( "plotFakeRates", flags.PhysVal.IDTPM.currentTrkAna.plotFakeRates )
    kwargs.setdefault( "unlinkedAsFakes", flags.PhysVal.IDTPM.currentTrkAna.unlinkedAsFakes )
    kwargs.setdefault( "plotDuplicateRates", flags.PhysVal.IDTPM.currentTrkAna.plotDuplicateRates )
    kwargs.setdefault( "plotHitsOnTracks", flags.PhysVal.IDTPM.currentTrkAna.plotHitsOnTracks )
    kwargs.setdefault( "plotHitsOnTracksReference", flags.PhysVal.IDTPM.currentTrkAna.plotHitsOnTracksReference )
    kwargs.setdefault( "plotHitsOnMatchedTracks", flags.PhysVal.IDTPM.currentTrkAna.plotHitsOnMatchedTracks )
    kwargs.setdefault( "plotHitsOnFakeTracks", flags.PhysVal.IDTPM.currentTrkAna.plotHitsOnFakeTracks )
    kwargs.setdefault( "plotVertexParameters", flags.PhysVal.IDTPM.currentTrkAna.plotVertexParameters )
    kwargs.setdefault( "useSelectedVertexTracks", flags.PhysVal.IDTPM.currentTrkAna.useSelectedVertexTracks )
    kwargs.setdefault( "plotOfflineElectrons", flags.PhysVal.IDTPM.currentTrkAna.plotOfflineElectrons )
    kwargs.setdefault( "ResolutionMethod", flags.PhysVal.IDTPM.currentTrkAna.ResolutionMethod )
    kwargs.setdefault( "isITk", flags.Detector.GeometryITk )
    kwargs.setdefault( "plotTracksInJets", "Jet" in flags.PhysVal.IDTPM.currentTrkAna.SelectOfflineObject )

    kwargs.setdefault("EtaBins", flags.Tracking.ITkMainPass.etaBins if flags.Detector.GeometryITk else [-1, 9999.]) # for technical efficiencies
    kwargs.setdefault("MinSilHits", flags.Tracking.ITkMainPass.minClusters if flags.Detector.GeometryITk else [flags.Tracking.MainPass.minClusters]) # for technical efficiencies

    trkAnaSvc = CompFactory.TrackAnalysisDefinitionSvc( name, **kwargs )
    acc.addService( trkAnaSvc )
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
