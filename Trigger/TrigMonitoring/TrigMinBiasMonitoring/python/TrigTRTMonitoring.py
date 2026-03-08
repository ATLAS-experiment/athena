#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
"""
@brief Configuration for the heavy-ion part of MinBias monitoring
"""

from .utils import getMinBiasChains
from AthenaCommon.Logging import logging

log = logging.getLogger('TrigTRTMonitoring')


def TrigTRTMonCfg(flags):
    from AthenaMonitoring import AthMonitorCfgHelper
    monConfig = AthMonitorCfgHelper(flags, 'TRTMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    alg = monConfig.addAlgorithm(CompFactory.TRTMonitoringAlg, 'TRTMonitoringAlg')

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_LoosePrimary_Cfg)
    trkSel = monConfig.resobj.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(flags))
    alg.TrackSelectionTool = trkSel

    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    monAccess = getHLTMonitoringAccess(flags)
    chains = getMinBiasChains(monAccess, 'HLT_noalg_L1TRT_FILLED|HLT_mb_excl_1trk5_pt1_hi_FgapAC5_L1TRT_VjTE20|HLT_noalg_L1TRT_VjTE50')

    ref_chains = ['HLT_mb_sptrk_L1ZDC_XOR_VjTE200', 'HLT_mb_sptrk_L1ZDC_1XOR5_VjTE200', 'HLT_mb_sptrk_L11ZDC_A_1ZDC_C_VjTE200'
    'HLT_mb_sptrk_hi_FgapC5_L1VjTE200', 'HLT_mb_sptrk_hi_FgapA5_L1VjTE200', 'HLT_noalg_L1RD0_FILLED', 'HLT_mb_sptrk_L1VjTE50']

    log.info(f'Monitoring {len(chains)} L1TRT chain(s)')
    log.debug([name for name, _ in chains])

    alg.triggerList = [name for name, _ in chains]
    alg.refTriggerList = ref_chains

    for chain, group in chains:
        hiTRTGroup = monConfig.addGroup(alg, f'{chain}', topPath=f'HLT/MinBiasMon/{group}/L1TRT/{chain}/')

        # 1D histograms
        hiTRTGroup.defineHistogram('n_trk', title='Track multiplicity;N_{trk};Events / 1', xbins=20, xmin=-0.5, xmax=19.5)
        hiTRTGroup.defineHistogram('lead_trk_pT', title='Leading track p_{T};p_{T}^{lead} [GeV];Events / 100 MeV', xbins=40, xmin=0, xmax=4)

        # TEfficiency
        hiTRTGroup.defineHistogram(f'effPassed,n_trk;{chain}_eff_ntrk', type='TEfficiency',
                                        title='L1 TRT efficiency;N_{trk};Efficiency', xbins=20, xmin=-0.5, xmax=19.5)
        hiTRTGroup.defineHistogram(f'effPassed,lead_trk_pT;{chain}_eff_pT', type='TEfficiency',
                                    title='L1 TRT efficiency;p_{T}^{lead} [GeV];Efficiency', xbins=40, xmin=0, xmax=4)

    return monConfig.result()


if __name__ == "__main__":
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.DQ.Environment = "AOD"
    flags.Concurrency.NumConcurrentEvents = 5

    flags.Output.HISTFileName = "TestTRTMonitorOutput.root"
    flags.fillFromArgs()
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    log.info("Input %s", str(flags.Input.Files))
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigTRTMonCfg(flags))

    from AthenaCommon.Constants import DEBUG
    cfg.getEventAlgo("TRTMonitoringAlg").OutputLevel = DEBUG
    cfg.printConfig(withDetails=True)
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()
    # to run:
    # python -m TrigMinBiasMonitoring.TrigTRTMonitoring --filesInput=...
