#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''
@brief configuration for the trigger efficiency monitoring
'''

from .utils import getMinBiasChains
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory

log = logging.getLogger('TrigMinBiasEffMonitoring')


def _TrigEff(flags, triggerAndRef, algname='HLTMinBiasEffMonitoringAlg'):
    from AthenaMonitoring import AthMonitorCfgHelper
    monConfig = AthMonitorCfgHelper(flags, algname)

    alg = monConfig.addAlgorithm(
        CompFactory.HLTMinBiasEffMonitoringAlg, algname)

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_LoosePrimary_Cfg)
    trkSel = monConfig.resobj.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(flags))
    alg.TrackSelectionTool = trkSel

    alg.triggerList = [ el["chain"] for el in  triggerAndRef ]
    alg.refTriggerList = [ el["refchain"] for el in  triggerAndRef ]

    length = len(alg.triggerList)

    mainGroup = monConfig.addGroup(alg, 'TrigAll', topPath='HLT/MinBiasMon/Counts/')

    alreadyConfigured = set()
    for cdef in triggerAndRef:
        chain = cdef['chain']
        refchain = cdef['refchain']
        level = cdef['level']
        xmin  = cdef['xmin']
        xmax  = cdef['xmax']
        xbins = xmax-xmin
        effGroup = monConfig.addGroup(alg, chain+refchain, topPath=f'HLT/MinBiasMon/{level}/EffAll/')

        whichcounter='nTrkOffline'
        # if the chain cuts on higher pt (there is a few predefined chains) use different counter
        if '_sptrk_pt' in chain:
            effGroup.defineHistogram(f'EffPassed,leadingTrackPt;{chain}_ref_{refchain}_pt', type='TEfficiency',
                                      title=chain+';Leading track pt;Efficiency', xbins=50, xmin=0.0, xmax=10)

            # these chains have such name pattern: HLT_mb_sptrk_pt2_L1MBTS_2
            whichcounter += '_'+chain.split('_')[3]
        elif '_excl_' in chain:
            effGroup.defineHistogram(f'EffPassed,nTrkOffline;{chain}_ref_{refchain}_exclusivity', type='TEfficiency',
                                                 title=chain+';Offline Good nTrk (low pt);Efficiency', xbins=30, xmin=-0.5, xmax=30-0.5)
            effGroup.defineHistogram(f'EffPassed,leadingTrackPt;{chain}_ref_{refchain}_pt', type='TEfficiency',
                                                  title=chain+';Leading track pt;Efficiency', xbins=50, xmin=0.0, xmax=10)
            # these chains have such form: HLT_mb_excl_1trk5_pt4_L1RD0_FILLED
            whichcounter += '_'+chain.split('_')[4]

        if '_pusup' in chain or '_hmt_' in chain:
            whichcounter = 'nTrkOfflineVtx'
        effGroup.defineHistogram(f'EffPassed,{whichcounter};{chain}_ref_{refchain}', type='TEfficiency',
                                    title=f'{chain} ref: {refchain} ;Offline Good nTrk {whichcounter};Efficiency', xbins=xbins, xmin=xmin, xmax=xmax)



        if chain not in alreadyConfigured:
            alreadyConfigured.add(chain)
            # need this protection because we can measure efficiency with several reference trigger, but want counts irrespective of ref. triggers
            mainGroup.defineHistogram('nTrkOffline_counts_' + chain, type='TH1F',
                                      title=chain+';Offline Good nTrk;Events', xbins=xmax-xmin, xmin=xmin, xmax=xmax)

    mainGroup.defineHistogram('TrigCounts', title='Trigger counts;;Event rate',
                              xbins=length, xmin=0, xmax=len(alreadyConfigured), xlabels=list(alreadyConfigured))

    return monConfig.result()


def TrigMinBiasEff(flags):
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    monAccess = getHLTMonitoringAccess(flags)

    mbNames = [name for name, _ in getMinBiasChains(monAccess)]
    if len(mbNames) == 0:
        return _TrigEff(flags, [])

    log.info(f'Monitoring {len(mbNames)} MinBias chains')
    log.debug(mbNames)

    # here we generate config with detailed settings
    def _c(chain, refchain, level, **kwargs):
        conf = {"chain": chain, "refchain": refchain, "level": level, "xmin": 0, "xmax": 20}
        conf.update(kwargs)
        return conf

    def _isFilled(chain):
        name, _ = chain
        return "_EMPTY" not in name and "_UNPAIRED_ISO" not in name

    spTrkChains = getMinBiasChains(monAccess, '(_sptrk_|_sp_|_mbts_)')
    filledChains = list(filter(_isFilled, spTrkChains))
    emptyChains = [chain for chain in spTrkChains if "_EMPTY" in chain[0]]
    unpairedChains = [chain for chain in spTrkChains if "_UNPAIRED_ISO" in chain[0]]

    triggerAndRef = []

    # check all mb_sptrk chains w.r.t. random noalg
    triggerAndRef += [_c(name, "HLT_noalg_L1RD0_FILLED", level) for name, level in filledChains]
    triggerAndRef += [_c(name, "HLT_noalg_L1RD0_EMPTY", level) for name, level in emptyChains]
    triggerAndRef += [_c(name, "HLT_noalg_L1RD0_UNPAIRED_ISO", level) for name, level in unpairedChains]

    # for monitoring in MB stream
    triggerAndRef += [_c(name, "HLT_noalg_mb_L1RD0_FILLED", level) for name, level in filledChains]
    triggerAndRef += [_c(name, "HLT_noalg_mb_L1RD0_EMPTY", level) for name, level in emptyChains]
    triggerAndRef += [_c(name, "HLT_noalg_mb_L1RD0_UNPAIRED_ISO", level) for name, level in unpairedChains]

    # sptrk vs sp
    triggerAndRef += [_c("HLT_mb_sptrk_L1RD0_FILLED", "HLT_mb_sp_L1RD0_FILLED", 'Shifter')]

    # HMT chains
    hmtChains = getMinBiasChains(monAccess, '(hmt)')
    nonPusupChains = [chain for chain in hmtChains if '_pusup' not in chain[0]]
    if len(nonPusupChains) != 0:
        # sort by trk threshold
        def _trk(chain):
            name, _ = chain
            part = name.split("_")
            for el in part:
                if el.startswith("trk"):
                    return int(el.strip("trk"))
            raise RuntimeError(f"Chain {name} is not the hmt chain")

        nonPusupChains.sort(key=lambda chain: int(_trk(chain)))

        # monitor first hmt w.r.t sptrk
        first = nonPusupChains[0]
        triggerAndRef += [_c(first[0], "HLT_mb_sptrk_L1RD0_FILLED", first[1], xmax=_trk(first) + 30)]

        # group set the ref for each trigger to be one of lower threshold : ordering of chains needs to be reviewed
        triggerAndRef += [_c(chain[0], "HLT_mb_sptrk_L1RD0_FILLED", chain[1], xmin=_trk(chain) - 20, xmax=_trk(chain) + 50) for chain in nonPusupChains[1:]]

        # pu suppressing trigger should be monitored using trigger of the same threshold w/o pu suppression
        pusupChains = [chain for chain in hmtChains if '_pusup' in chain[0]]

        def _dropsup(name):
            s = name.split("_")
            return "_".join(s[:3] + s[4:])

        triggerAndRef += [_c(chain[0], _dropsup(chain[0]), chain[1], xmin=_trk(chain) - 20, xmax=_trk(chain) + 50) for chain in pusupChains]

    # monitor exclusivity cut
    exclChains = getMinBiasChains(monAccess, '(excl)')
    for name, level in exclChains:
        triggerAndRef.append(_c(name, 'HLT_mb_sptrk_L1RD0_FILLED', level))
        triggerAndRef.append(_c(name, 'HLT_mb_sp_L1RD0_FILLED', level))

    # monitor noalg MBTS chains
    noalgMbtsChains = getMinBiasChains(monAccess, '^HLT_noalg_.*(L1MBTS)')
    triggerAndRef += [_c(name, 'HLT_mb_sptrk_L1RD0_FILLED', level) for name, level in noalgMbtsChains]

    # L1 MBTS
    mbtsL1Chains = ["L1_MBTS_A", "L1_MBTS_C", "L1_MBTS_1", "L1_MBTS_2", "L1_MBTS_1_1"]
    triggerAndRef += [_c(chain, 'HLT_mb_sptrk_L1RD0_FILLED', 'Expert') for chain in mbtsL1Chains]

    # L1 transverse energy
    triggerAndRef += [_c("L1_TE{}".format(i), 'HLT_mb_sptrk_L1RD0_FILLED', 'Expert', xmin=0, xmax=100) for i in [3, 5, 10, 40]]

    # Pair HLT chain with its noalg version
    def _find_noalg(chain):
        pos = chain.find('L1')
        return 'HLT_noalg_' + chain[pos:]

    def _chains_with_noalg_ref():
        chains = []

        for name, level in filledChains:
            # Already added
            if 'L1RD0_FILLED' in name:
                continue

            noalg = _find_noalg(name)
            if noalg not in mbNames:
                continue

            chains.append(_c(name, noalg, level))

        return chains

    triggerAndRef += _chains_with_noalg_ref()

    # HI chains
    hiChains = getMinBiasChains(monAccess, '(_hi_)')

    # Pair MB+HI chains with MB only reference if it exists
    def _hi_chain_with_mb_ref():
        chains = []

        for name, level in hiChains:
            split = name.split('_hi_')
            if len(split) != 2:
                continue
            mb, hi_l1 = split

            split = hi_l1.split('L1')
            if len(split) != 2:
                continue
            hi, l1 = split

            ref = f'{mb}_L1{l1}'
            if ref not in mbNames:
                continue

            chains.append(_c(name, ref, level))

        return chains

    triggerAndRef += _hi_chain_with_mb_ref()

    # Add here all the special cases:
    # HI Fgap chains
    triggerAndRef.append(_c('HLT_mb_excl_1trk4_pt1_hi_FgapAC5_L12eEM1_VjTE200', 'HLT_mb_excl_1trk4_pt1_L12eEM1_VjTE200_GAP_AANDC', 'Expert'))

    return _TrigEff(flags, triggerAndRef)


if __name__ == '__main__':
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Output.HISTFileName = 'TestMinBiasMonitorOutput.root'
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigMinBiasEff(flags))


    #    cfg.getEventAlgo('HLTMinBiasEffMonitoringAlg').OutputLevel = DEBUG  # DEBUG
    cfg.printConfig(withDetails=True)  # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()
    # to run:
    # python -m TrigMinBiasMonitoring.TrigMinBiasEffMonitoring --filesInput=filepath --evtMax=XYZ
