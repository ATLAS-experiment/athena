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
    acc = ComponentAccumulator()

    kwargs.setdefault( "DirName", flags.PhysVal.IDTPM.DirName )
    kwargs.setdefault( "sortPlotsByChain", flags.PhysVal.IDTPM.sortPlotsByChain )
    kwargs.setdefault( "SubFolder", flags.PhysVal.IDTPM.currentTrkAna.SubFolder )
    kwargs.setdefault( "TrkAnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    kwargs.setdefault( "TestType", flags.PhysVal.IDTPM.currentTrkAna.TestType )
    kwargs.setdefault( "RefType",  flags.PhysVal.IDTPM.currentTrkAna.RefType )

    from InDetTrackPerfMon.ConfigUtils import getTag
    kwargs.setdefault( "TestTag", getTag( flags, flags.PhysVal.IDTPM.currentTrkAna.TestType ) )
    kwargs.setdefault( "RefTag",  getTag( flags, flags.PhysVal.IDTPM.currentTrkAna.RefType ) )

    kwargs.setdefault( "MatchingType", flags.PhysVal.IDTPM.currentTrkAna.MatchingType )
    kwargs.setdefault( "MatchingTruthProb", flags.PhysVal.IDTPM.currentTrkAna.truthProbCut )

    if ( ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType and  "EFTrigger" not in flags.PhysVal.IDTPM.currentTrkAna.TestType) or
         ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.RefType and "EFTrigger" not in flags.PhysVal.IDTPM.currentTrkAna.RefType ) ):
        kwargs.setdefault( "ChainNames", flags.PhysVal.IDTPM.currentTrkAna.ChainNames )

    kwargs.setdefault( "plotTrackParameters", flags.PhysVal.IDTPM.currentTrkAna.plotTrackParameters )
    kwargs.setdefault( "plotEfficiencies", flags.PhysVal.IDTPM.currentTrkAna.plotEfficiencies )
    kwargs.setdefault( "plotResolutions", flags.PhysVal.IDTPM.currentTrkAna.plotResolutions )
    kwargs.setdefault( "plotFakeRates", flags.PhysVal.IDTPM.currentTrkAna.plotFakeRates )
    kwargs.setdefault( "plotOfflineElectrons", flags.PhysVal.IDTPM.currentTrkAna.plotOfflineElectrons )

    trkAnaSvc = CompFactory.TrackAnalysisDefinitionSvc( name, **kwargs )
    acc.addService( trkAnaSvc )
    return acc


def InDetTrackPerfMonToolCfg( flags, name="InDetTrackPerfMonTool", **kwargs ):
    '''
    Main IDTPM tool instance CA-based configuration
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "OfflineTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.OfflineTrkKey )
    kwargs.setdefault( "TruthParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TruthPartKey )

    kwargs.setdefault( "AnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

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

    ## now the sub-tools    
    if "TrackQualitySelectionTool" not in kwargs:
        from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
        kwargs.setdefault( "TrackQualitySelectionTool", acc.popToolsAndMerge(
            TrackQualitySelectionToolCfg( flags,
                name="TrackQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if "Truth" in flags.PhysVal.IDTPM.currentTrkAna.RefType or "Truth" in flags.PhysVal.IDTPM.currentTrkAna.TestType:
        from InDetTrackPerfMon.InDetAlgorithmConfig import TruthHitDecoratorAlgCfg
        acc.merge( TruthHitDecoratorAlgCfg(flags) )

    if ( ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType ) or
         ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.RefType ) ):

        kwargs.setdefault( "TriggerTrkParticleContainerName",
                           flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )

        if ( "EFTrigger" not in flags.PhysVal.IDTPM.currentTrkAna.TestType and "EFTrigger" not in flags.PhysVal.IDTPM.currentTrkAna.RefType ):
            if "TrigDecisionTool" not in kwargs:
                from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
                kwargs.setdefault( "TrigDecisionTool", 
                                acc.getPrimaryAndMerge( TrigDecisionToolCfg(flags) ) )

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

    if "TrackMatchingTool" not in kwargs:
        from InDetTrackPerfMon.InDetMatchingConfig import TrackMatchingToolCfg
        matchToolCfg = TrackMatchingToolCfg( flags )
        if matchToolCfg is not None :
            kwargs.setdefault( "doMatch", True ) # = False by default
            kwargs.setdefault( "TrackMatchingTool", acc.popToolsAndMerge( matchToolCfg ) )

    acc.setPrivateTools( CompFactory.InDetTrackPerfMonTool( name, **kwargs ) )
    return acc


def InDetTrackPerfMonCfg( flags ):
    '''
    CA-based configuration of all tool instances (= TrackAnalyses)
    '''
    log = logging.getLogger( "InDetTrackPerfMonCfg" )
    acc = ComponentAccumulator()

    ## Offline track-object decorator
    useOfflineObject = False
    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames :
        obj = getattr( flags.PhysVal.IDTPM, trkAnaName+".SelectOfflineObject" )
        if ( not obj or "Truth" in obj ) :
            # Do not schedule algorithm
            # if SelectOfflineObject id empty or
            # for Truth-match offline selection
            continue
        useOfflineObject = True
        break

    if useOfflineObject:
        from InDetTrackPerfMon.InDetAlgorithmConfig import OfflineObjectDecoratorAlgCfg
        acc.merge( OfflineObjectDecoratorAlgCfg(flags) )

    ## IDTPM tool instances
    tools = []

    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames :
        ## cloning flags of current TrackAnalysis to PhysVal.IDTPM.currentTrkAna
        flags_thisTrkAna = flags.cloneAndReplace( "PhysVal.IDTPM.currentTrkAna",
                                                  "PhysVal.IDTPM."+trkAnaName )

        if flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.enabled:
            log.debug( "Scheduling TrackAnalysis: %s",
                       flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.anaTag )

            tools.append(
                acc.popToolsAndMerge( InDetTrackPerfMonToolCfg( flags_thisTrkAna,
                    name="InDetTrackPerfMonTool"+
                         flags_thisTrkAna.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    acc.merge( PhysValMonitoringCfg( flags, tools=tools ) )

    ## Adding additional output stream for reprocessing file
    if flags.Output.doWriteAOD_IDTPM :
        from InDetTrackPerfMon.InDetOutputConfig import InDetOutputCfg
        acc.merge( InDetOutputCfg(flags) )

    return acc
