#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

def TauMonitoringConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # the following should not run in RAW to ESD, if we're in two-step
    if flags.DQ.Environment != 'tier0Raw':

        # We need the TauID inference to run first
        from AthenaCommon.CFElements import seqAND
        seq_name = 'TauMonitoringSeq'
        result.addSequence(seqAND(seq_name))

        # Schedule the offline GNTau inference when running from AOD (GNTau is now transiently available in RAWtoALL)
        if flags.DQ.Environment == 'AOD':
            TauContainerCopy = 'TauMonTauJets'

            from tauRec.TauToolHolder import TauVertexedClusterDecoratorCfg, TauGNNEvaluatorCfg, TauWPDecoratorGNNCfg
            tool_accs = [
                TauVertexedClusterDecoratorCfg(flags),
                TauGNNEvaluatorCfg(flags, 0, tauContainerName=TauContainerCopy),
                TauWPDecoratorGNNCfg(flags, 0, TauContainerCopy),
            ]

            tools = []
            for tool_acc in tool_accs:
                tools.append(tool_acc.popPrivateTools())
                tools[-1].inAOD = True
                result.merge(tool_acc, seq_name)
                result.addPublicTool(tools[-1])

                from AthenaConfiguration.ComponentFactory import CompFactory
                result.addEventAlgo(CompFactory.TauAODRunnerAlg(
                    name='TauMonitoring_TauJets_TauIDDecorator',
                    Key_tauContainer='TauJets',
                    Key_pi0ClusterInputContainer='',
                    Key_tauOutputContainer=TauContainerCopy,
                    Key_pi0OutputContainer='',
                    Key_neutralPFOOutputContainer='',
                    Key_chargedPFOOutputContainer='',
                    Key_hadronicPFOOutputContainer='',
                    Key_tauTrackOutputContainer='',
                    Key_vertexOutputContainer='',
                    officialTools=tools,
               ), sequenceName=seq_name)

        from .tauMonitorAlgorithm import tauMonitoringConfig

        if flags.DQ.Environment == 'AOD':
            offline_taujets = 'TauMonTauJets'
        else:
            offline_taujets = 'TauJets'

        result.merge(tauMonitoringConfig(flags,tauContainer=offline_taujets))

        

    return result
