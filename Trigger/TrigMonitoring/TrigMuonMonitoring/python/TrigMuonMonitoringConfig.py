#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from TrigMuonMonitoring.L1MuonMonConfig import L1MuonMonConfig
from TrigMuonMonitoring.L2MuonSAMonConfig import L2MuonSAMonConfig
from TrigMuonMonitoring.L2MuonSAIOMonConfig import L2MuonSAIOMonConfig
from TrigMuonMonitoring.L2muCombMonConfig import L2muCombMonConfig
from TrigMuonMonitoring.L2OverlapRemoverMonConfig import L2OverlapRemoverMonConfig
from TrigMuonMonitoring.EFMuonMonConfig import EFMuonMonConfig
from TrigMuonMonitoring.TrigMuonEfficiencyMonConfig import TrigMuonEfficiencyMonTTbarConfig, TrigMuonEfficiencyMonZTPConfig
from TrigMuonMonitoring.MuonTriggerCountConfig import MuonTriggerCountConfig
from TrigMuonMonitoring.TrigMuonTruthMonConfig import TrigMuonTruthMonConfig


def TrigMuonMonConfig(inputFlags):

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigMuonMonitoringCfg')

    from TriggerMenuMT.HLT.Muon.TrigMuonKeys import muonNames
    isPhaseIIalgs = (muonNames().getNames('RoI').EFSATrackParticlesPhII in inputFlags.Input.Collections) or inputFlags.Trigger.Offline.SA.Muon.scheduleActsReco

    L1MuonMonConfig(helper, isPhaseIIalgs)
    L2MuonSAMonConfig(helper, isPhaseIIalgs)
    L2MuonSAIOMonConfig(helper, isPhaseIIalgs)
    L2muCombMonConfig(helper, isPhaseIIalgs)
    L2OverlapRemoverMonConfig(helper, isPhaseIIalgs)
    EFMuonMonConfig(helper, isPhaseIIalgs)
    TrigMuonEfficiencyMonTTbarConfig(helper, isPhaseIIalgs)
    TrigMuonEfficiencyMonZTPConfig(helper, isPhaseIIalgs)
    MuonTriggerCountConfig(helper, isPhaseIIalgs)
    if inputFlags.Input.isMC:
        TrigMuonTruthMonConfig(helper, isPhaseIIalgs)

    return helper.result()

if __name__=='__main__':     
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/'
    aodfile = 'data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1'
    flags = initConfigFlags()
    flags.Input.Files = [nightly+aodfile]
    flags.Input.isMC = True
    flags.Output.HISTFileName = 'HIST.root'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    muonMonitorAcc = TrigMuonMonConfig(flags)
    cfg.merge(muonMonitorAcc)

    cfg.printConfig(withDetails=False)
    cfg.run()
