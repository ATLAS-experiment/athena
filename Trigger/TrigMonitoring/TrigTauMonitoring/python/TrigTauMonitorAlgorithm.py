#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def TrigTauMonConfig(flags):
    '''Function to configures some algorithms in the monitoring system.'''
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    # We need the TauID inference to run first
    from AthenaCommon.CFElements import seqAND
    seq_name = 'TrigTauMonitoringSeq'
    acc.addSequence(seqAND(seq_name))


    # Schedule the offline GNTau inference
    if flags.Reco.EnableTau:
        from tauRec.TauToolHolder import TauVertexedClusterDecoratorCfg, TauGNNEvaluatorCfg, TauWPDecoratorGNNCfg
        tool_accs = [
            TauVertexedClusterDecoratorCfg(flags),
            TauGNNEvaluatorCfg(flags, 0),
            TauWPDecoratorGNNCfg(flags, 0),
        ]

        tools = []
        for tool_acc in tool_accs:
            tools.append(tool_acc.popPrivateTools())
            tools[-1].inAOD = True
            acc.merge(tool_acc, seq_name)
            acc.addPublicTool(tools[-1])

            from AthenaConfiguration.ComponentFactory import CompFactory
            acc.addEventAlgo(CompFactory.TauAODRunnerAlg(
                name='TrigTauMonitoring_TauJets_TauIDDecorator',
                Key_tauContainer='TauJets',
                Key_pi0ClusterInputContainer='',
                Key_tauOutputContainer='TTMTauJets',
                Key_pi0OutputContainer='',
                Key_neutralPFOOutputContainer='',
                Key_chargedPFOOutputContainer='',
                Key_hadronicPFOOutputContainer='',
                Key_tauTrackOutputContainer='',
                Key_vertexOutputContainer='',
                officialTools=tools,
            ), sequenceName=seq_name)


    # The following class will make a sequence, configure the base monitoring infrastructure
    # and algorithms, and link them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TrigTauAthMonitorCfg')

    # Configure the actual TrigTauMonitoring algorithm(s)
    from TrigTauMonitoring.TrigTauMonitoringConfig import TrigTauMonAlgBuilder
    monAlgCfg = TrigTauMonAlgBuilder(helper)
    monAlgCfg.offline_taujets = 'TTMTauJets'
    monAlgCfg.configure()
    acc.merge(helper.result(), seq_name)

    return acc


if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/'
    file = 'data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1'
    flags = initConfigFlags()
    flags.Input.Files = [nightly+file]
    flags.Input.isMC = False
    flags.Output.HISTFileName = 'TrigTauMonitorOutput.root'
    
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    trigTauMonitorAcc = TrigTauMonConfig(flags)
    cfg.merge(trigTauMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigJetMonitorAcc.getEventAlgo('TrigTauMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    sc = cfg.run() #use cfg.run(20) to only run on first 20 events
    if not sc.isSuccess():
       import sys
       sys.exit("Execution failed")


