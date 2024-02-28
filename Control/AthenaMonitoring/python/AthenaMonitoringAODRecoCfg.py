# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def AthenaMonitoringAODRecoCfg(flags):
    import logging
    local_logger = logging.getLogger('AthenaMonitoringAODRecoCfg')
    info = local_logger.info
    result = ComponentAccumulator()

    if flags.DQ.Environment == 'AOD':
        info('Running on AOD: Scheduling rebuild of standard jet collections if necessary')
        from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo, AntiKt4EMPFlow, AntiKt4LCTopo
        from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_noVR, AntiKt10LCTopoTrimmed_trigger, AntiKt10UFOCSSKSoftDrop_trigger
        from AthenaConfiguration.Enums import BeamType

        # first, check small R jets, skip PFlow when running over cosmics
        jets_to_schedule = [_ for _ in ((AntiKt4EMTopo, AntiKt4EMPFlow, AntiKt4LCTopo) if flags.Beam.Type is not BeamType.Cosmics else (AntiKt4EMTopo, AntiKt4LCTopo))
                            if _.fullname() not in flags.Input.Collections]
        # if we reschedule small R jets, check if we need to reschedule large R as well
        if jets_to_schedule:
            jets_to_schedule += [_ for _ in ((AntiKt10LCTopo_noVR, AntiKt10LCTopoTrimmed_trigger, AntiKt10UFOCSSKSoftDrop_trigger) if flags.Beam.Type is not BeamType.Cosmics else (AntiKt10LCTopo_noVR, AntiKt10LCTopoTrimmed_trigger))
                                 if _.fullname() not in flags.Input.Collections]

        if jets_to_schedule:
            info(f'Ensuring presence of jet collections for monitoring: {jets_to_schedule}')
            from JetRecConfig.JetRecConfig import JetRecCfg
            from JetRecConfig.JetConfigFlags import jetInternalFlags            
            # We're in Reco-like job : this flag implies the jet config will automatically switch off component incompatible with conditions (cosmics, truth,...)
            jetInternalFlags.isRecoJob = True
            
            for container in jets_to_schedule:
                result.merge(JetRecCfg(flags, container))

        jet_collections = set([_.fullname().replace('Jets','') for _ in jets_to_schedule])
        btag_jet_collections = set(['AntiKt4EMTopo', 'AntiKt4EMPFlow'])
        met_jet_collections = set(['AntiKt4EMTopo', 'AntiKt4EMPFlow', 'AntiKt4LCTopo'])

        if jet_collections & btag_jet_collections:
            info('Scheduling b-tagging of rebuilt jets')
            from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
            result.merge(BeamSpotCondAlgCfg(flags))
            from BTagging.BTagConfig import BTagRecoSplitCfg
            # would rather use removesuffix below but need to wait for Python 3.9
            for container in jet_collections & btag_jet_collections:
                result.merge(BTagRecoSplitCfg(flags, [container]))

        # MET can't be rebuilt when running over cosmics or HI AOD as taus are missing
        from .DQConfigFlags import DQDataType
        if jet_collections & met_jet_collections and flags.DQ.DataType not in (DQDataType.HeavyIon, DQDataType.Cosmics):
            info('Scheduling rebuild of standard MET')
            from METReconstruction.METAssociatorCfg import METAssociatorCfg
            from METUtilities.METMakerConfig import getMETMakerAlg
            for container in jet_collections & met_jet_collections:
                if container == 'AntiKt4EMPFlow':
                    # build links between FlowElements and electrons, photons, muons and taus                    
                    info('Scheduling FlowElement linking')
                    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
                    result.merge(PFGlobalFlowElementLinkingCfg(flags))
                result.merge(METAssociatorCfg(flags, container))
                result.addEventAlgo(getMETMakerAlg(container))
            from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
            result.merge(CaloNoiseCondAlgCfg(flags)) # Prereq for Calo MET
            from METReconstruction.METCalo_Cfg import METCalo_Cfg
            result.merge(METCalo_Cfg(flags))

    return result
