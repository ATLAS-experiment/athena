#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
"""
@brief Configuration for the heavy-ion part of MinBias monitoring
"""

from .utils import getMinBiasChains
from AthenaCommon.Logging import logging

log = logging.getLogger('TrigHIMonitoringMT')


def split_num(l1: str):
    head = l1.rstrip('0123456789')
    tail = l1[len(head):]
    return head.replace('j', ''), int(tail)


def ranges_from_chain(chain):
    _, l1 = chain.split('L1')

    # Extract (Vj)TE items and separate numbers from them, e.g.: 'VjTE200' -> {'VTE': 200}
    l1_te_items = dict(map(split_num, filter(lambda x: 'TE' in x and 'TEA' not in x, l1.split('_'))))

    xmax = l1_te_items.pop('VTE', 20e3 if 'hi_ucc' in chain else 100) * 1.2
    # TODO: xmin from (j)TE?

    xbins = xmax if 'hi_ucc' not in chain else 240
    if xbins < 50:
        xbins *= 2

    ranges = {'xbins': int(xbins), 'xmin': 0., 'xmax': xmax}
    ranges_fwd = {'xbins': int(xbins), 'xmin': 0., 'xmax': xmax / 3.}

    return ranges, ranges_fwd


def TrigHIMonCfg(flags):
    from AthenaMonitoring import AthMonitorCfgHelper
    monConfig = AthMonitorCfgHelper(flags, 'HLTHeavyIonMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    alg = monConfig.addAlgorithm(CompFactory.HLTHeavyIonMonAlg, 'HLTHeavyIonMonAlgo')

    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    monAccess = getHLTMonitoringAccess(flags)
    hiChains = getMinBiasChains(monAccess, '(hi_Fgap|hi_ucc).*(TE)')

    # For testing use all chains
    # from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
    # hiChains = [(c, 'Expert') for c in getHLTMenuAccess(flags) if ('hi_Fgap' in c or 'hi_ucc' in c) and 'TE' in c]

    log.info(f'Monitoring {len(hiChains)} MinBias heavy-ion chains')
    log.debug([name for name, _ in hiChains])

    alg.triggerListMon = [name for name, _ in hiChains]

    for chain, group in hiChains:
        hiSumETGroup = monConfig.addGroup(alg, f'{chain}_sumEt', topPath=f'HLT/MinBiasMon/{group}/SumEt/{chain}/')

        # 1D histograms
        ranges, ranges_fwd = ranges_from_chain(chain)
        hiSumETGroup.defineHistogram('sum_L1TE', title='TE sum from legacy L1;L1 Legacy E_{T} [GeV];Events', **ranges)
        hiSumETGroup.defineHistogram('sum_L1jTE', title='TE sum from jFex L1;L1 jFEX E_{T} [GeV];Events', **ranges)
        hiSumETGroup.defineHistogram('sum_L1FWDAjTE', title='FwdTE sum from jFex L1 (side A);L1 jFEX FWD_{A} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_L1FWDCjTE', title='FwdTE sum from jFex L1 (side C);L1 jFEX FWD_{C} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_FCalAEt', title='FCal sum from HLT (side A);HLT FCal_{A} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_FCalCEt', title='FCal sum from HLT (side C);HLT FCal_{C} E_{T} [GeV];Events', **ranges_fwd)

        # 2D histograms
        ranges |= {'ybins': ranges['xbins'], 'ymin': ranges['xmin'], 'ymax': ranges['xmax']}
        ranges_fwd |= {'ybins': ranges_fwd['xbins'], 'ymin': ranges_fwd['xmin'], 'ymax': ranges_fwd['xmax']}
        hiSumETGroup.defineHistogram('sum_L1TE,sum_L1jTE', type='TH2F', title='Legacy vs Phase-I TE;L1 Legacy E_{T} [GeV];L1 jFEX E_{T} [GeV];Events', **ranges)
        hiSumETGroup.defineHistogram('sum_L1FWDAjTE,sum_L1FWDCjTE', type='TH2F', title='FwdTE A-C side corr.;L1 jFEX FWD_{A} E_{T} [GeV];L1 jFEX FWD_{C} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_FCalAEt,sum_FCalCEt', type='TH2F', title='FCal sum A-C side corr.;HLT FCal_{A} E_{T} [GeV];HLT FCal_{C} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_L1FWDAjTE,sum_FCalAEt', type='TH2F', title='FwdTE vs FCal, A side;L1 jFEX FWD_{A} E_{T} [GeV];HLT FCal_{A} E_{T} [GeV];Events', **ranges_fwd)
        hiSumETGroup.defineHistogram('sum_L1FWDCjTE,sum_FCalCEt', type='TH2F', title='FwdTE vs FCal, C side;L1 jFEX FWD_{C} E_{T} [GeV];HLT FCal_{C} E_{T} [GeV];Events', **ranges_fwd)

    return monConfig.result()


if __name__ == "__main__":
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.DQ.Environment = "AOD"
    flags.Concurrency.NumConcurrentEvents = 5

    flags.Output.HISTFileName = "TestHIMonitorOutput.root"
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
    cfg.merge(TrigHIMonCfg(flags))

    # from AthenaCommon.Constants import DEBUG
    # cfg.getEventAlgo("HLTHeavyIonMonitoring").OutputLevel = DEBUG
    cfg.printConfig(withDetails=True)
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()
    # to run:
    # python -m TrigMinBiasMonitoring.TrigHIMonitoringMT --filesInput=...
