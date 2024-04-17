#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetTrackPerfMonConfig.py
@author M. Aparo
@date 2023-02-17
@brief Main CA-based python configuration for InDetTrackPerfMonTool
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


def TrackAnalysisDefinitionSvcCfg( flags, name="TrkAnaDefSvc", **kwargs ):
    '''
    CA-based configuration for the TrackAnalysisDefinition Service
    '''
    acc = ComponentAccumulator()

    kwargs.setdefault( "SubFolder", flags.PhysVal.IDTPM.currentTrkAna.SubFolder )
    kwargs.setdefault( "TrkAnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    kwargs.setdefault( "TestType", flags.PhysVal.IDTPM.currentTrkAna.TestType )
    kwargs.setdefault( "RefType",  flags.PhysVal.IDTPM.currentTrkAna.RefType )

    ## TODO - to be uncommented in future MRs
    #kwargs.setdefault( "TestTag", getTrkTag(flags.PhysVal.IDTPM.currentTrkAna.TestType))
    #kwargs.setdefault( "RefTag",  getTrkTag(flags.PhysVal.IDTPM.currentTrkAna.RefType))

    kwargs.setdefault( "MatchingType", flags.PhysVal.IDTPM.currentTrkAna.MatchingType )

    if ( ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType ) or
         ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.RefType ) ):
        kwargs.setdefault( "ChainNames", flags.PhysVal.IDTPM.currentTrkAna.ChainNames )

    kwargs.setdefault( "doTrackParameters", flags.PhysVal.IDTPM.currentTrkAna.doTrackParameters )
    kwargs.setdefault( "doEfficiencies", flags.PhysVal.IDTPM.currentTrkAna.doEfficiencies )
    kwargs.setdefault( "doOfflineElectrons", flags.PhysVal.IDTPM.currentTrkAna.doOfflineElectrons )

    trkAnaSvc = CompFactory.TrackAnalysisDefinitionSvc( name, **kwargs )
    acc.addService( trkAnaSvc )
    return acc


def InDetTrackPerfMonToolCfg( flags, name="InDetTrackPerfMonTool", **kwargs ):
    '''
    Main IDTPM tool instance CA-based configuration
    '''
    acc = ComponentAccumulator()

    ## TODO - to be uncommented in future MRs
    #acc.merge(HistogramDefinitionSvcCfg(flags, name="HistoDefSvc"+
    #                                flags.PhysVal.IDTPM.currentTrkAna.anaTag))

    kwargs.setdefault( "OfflineTrkParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.OfflineTrkKey )
    kwargs.setdefault( "TruthParticleContainerName",
                       flags.PhysVal.IDTPM.currentTrkAna.TruthPartKey )

    kwargs.setdefault( "DirName", flags.PhysVal.IDTPM.DirName )
    kwargs.setdefault( "AnaTag", flags.PhysVal.IDTPM.currentTrkAna.anaTag )

    acc.merge( TrackAnalysisDefinitionSvcCfg( flags,
                   name="TrkAnaDefSvc"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) )

    if "TrackQualitySelectionTool" not in kwargs:
        from InDetTrackPerfMon.InDetSelectionConfig import TrackQualitySelectionToolCfg
        kwargs.setdefault( "TrackQualitySelectionTool", acc.popToolsAndMerge(
            TrackQualitySelectionToolCfg( flags,
                name="TrackQualitySelectionTool"+flags.PhysVal.IDTPM.currentTrkAna.anaTag ) ) )

    if ( ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.TestType ) or
         ( "Trigger" in flags.PhysVal.IDTPM.currentTrkAna.RefType ) ):

        kwargs.setdefault( "TriggerTrkParticleContainerName",
                           flags.PhysVal.IDTPM.currentTrkAna.TrigTrkKey )

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

    ## Truth-hit decorator
    useTruth = False
    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames :
        if ( ( "Truth" in getattr( flags.PhysVal.IDTPM, trkAnaName+".TestType" ) ) or
             ( "Truth" in getattr( flags.PhysVal.IDTPM, trkAnaName+".RefType" ) ) ):
            useTruth = True
            break

    if useTruth:
        from InDetTrackPerfMon.InDetAlgorithmConfig import TruthHitDecoratorAlgCfg
        acc.merge( TruthHitDecoratorAlgCfg(flags) )

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
