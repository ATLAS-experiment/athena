#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@brief Configuration for the MBTS monitoring
'''
from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
from AthenaCommon.Logging import logging

from .utils import getMinBiasChains

log = logging.getLogger('TrigMBTSMonitoringMT')


def TrigMBTS(configFlags):
    from AthenaMonitoring import AthMonitorCfgHelper
    monConfig = AthMonitorCfgHelper(configFlags, 'HLTMBTSMonitoringAlgMTflags')

    from AthenaConfiguration.ComponentFactory import CompFactory
    alg = monConfig.addAlgorithm(
        CompFactory.HLTMBTSMonitoringAlgMT, 'HLTMBTSMonitoringAlgMT')

    monAccess = getHLTMonitoringAccess(configFlags)
    mbts_chains = getMinBiasChains(monAccess, '(_mbts_)')

    log.info(f'Monitoring {len(mbts_chains)} MBTS chains')
    log.debug([name for name, _ in mbts_chains])

    alg.triggerList = [name for name, _ in mbts_chains]
    channelLabels = [f'A{i:0>2d}' for i in range(16)]
    channelLabels += [f'C{i:0>2d}' for i in range(16)]

    length = len(alg.triggerList)
    MBTS_countsSideA = 16
    MBTS_countsSideC = 16

    mbtsGroup = monConfig.addGroup(alg, 'MBTSall', topPath='HLT/MBTSMon/')
    mbtsGroup.defineHistogram('TrigCounts', title='Trigger counts;;Event rate',
                              xbins=length, xmin=0, xmax=length, xlabels=list(alg.triggerList))

    for chain, level in mbts_chains:

        mbShiftGroup = monConfig.addGroup(alg, chain+'_shifter', topPath=f'HLT/MBTSMon/{level}/{chain}/')

        mbShiftGroup.defineHistogram('MBTS_A_hits', type='TH1I', title='MBTS hits side A; Entry rate; MBTS side A',
                                     xbins=MBTS_countsSideA+1, xmin=-0.5, xmax=MBTS_countsSideA+0.5)
        mbShiftGroup.defineHistogram('MBTS_C_hits', type='TH1I', title='MBTS hits side C; Entry rate; MBTS side C',
                                     xbins=MBTS_countsSideC+1, xmin=-0.5, xmax=MBTS_countsSideC+0.5)
        mbShiftGroup.defineHistogram('MBTS_A_EWTime,MBTS_C_EWTime', type='TH2I', title='MBTS timing correlation; MBTS side A; MBTS side C',
                                     xbins=40, xmin=-80, xmax=80, ybins=40, ymin=-80, ymax=80)
        mbShiftGroup.defineHistogram('MBTS_A_hits,MBTS_C_hits', type='TH2I', title='MBTS hits correlation; MBTS side A; MBTS side C',
                                     xbins=MBTS_countsSideA+1, xmin=-0.5, xmax=MBTS_countsSideA+0.5, ybins=MBTS_countsSideC+1, ymin=-0.5, ymax=MBTS_countsSideC+0.5)

        mbShiftGroup.defineHistogram(
            'MBTS_time', type='TH1F', title='MBTS time in all pads; MBTS time [ns]', xbins=100, xmin=-100, xmax=100)
        mbShiftGroup.defineHistogram(
            'MBTS_energy', type='TH1F', title='MBTS energy in all pads; MBTS energy [pC]', xbins=100, xmin=0.1, xmax=9.9)
        mbShiftGroup.defineHistogram('MBTS_channelID;MBTS_counts_per_channel', type='TH1F', title='Counts per MBTS Channel ID; Channel ID; Count',
                                     xbins=MBTS_countsSideA+MBTS_countsSideC, xmin=0, xmax=MBTS_countsSideA+MBTS_countsSideC, xlabels=list(channelLabels))
        mbShiftGroup.defineHistogram('MBTS_channelID,MBTS_time', type='TH2F', title='MBTS time; Channel ID;MBTS time [ns]; Entry rate',
                                     xbins=MBTS_countsSideA+MBTS_countsSideC, xmin=0, xmax=MBTS_countsSideA+MBTS_countsSideC, xlabels=list(channelLabels), ybins=100, ymin=-100, ymax=100)
        mbShiftGroup.defineHistogram('MBTS_channelID,MBTS_time;MBTS_time_zoom_vs_MBTS_channel_ID', type='TH2F', title='MBTS time zoom; Channel ID;MBTS time [ns]; Entry rate',
                                     xbins=MBTS_countsSideA+MBTS_countsSideC, xmin=0, xmax=MBTS_countsSideA+MBTS_countsSideC, xlabels=list(channelLabels), ybins=100, ymin=-50, ymax=50)
        mbShiftGroup.defineHistogram('MBTS_channelID,MBTS_energy', type='TH2F', title='MBTS energy; Channel ID;MBTS energy [pC]; Entry rate',
                                     xbins=MBTS_countsSideA+MBTS_countsSideC, xmin=0, xmax=MBTS_countsSideA+MBTS_countsSideC, xlabels=list(channelLabels), ybins=100, ymin=-0.1, ymax=1.9)

        mbShiftGroup.defineHistogram('MBTS_diff_timeMean', type='TH1F', title='Mean MBTS time difference between sides; MBTS time [ns]',
                                     xbins=100, xmin=-100, xmax=100)
        mbShiftGroup.defineHistogram('MBTS_diff_weightedTimeMean', type='TH1F', title='Energy weighted mean MBTS time difference between sides; MBTS time [ns]',
                                     xbins=100, xmin=-100, xmax=100)

        mbExpGroup = monConfig.addGroup(alg, chain+'_expert',
                                        topPath='HLT/MBTSMon/'+chain+'/')
        mbExpGroup.defineHistogram(
            'MBTShits', type='TH1F', title='MBTS total hits;# of Hits;Entry Rate', xbins=24, xmin=0, xmax=100)

        mbExpGroup.defineHistogram('MBTS_A_meanEnergy', type='TH1F',
                                   title='Mean MBTS Energy A side passed;MBTS Energy [pC];Entry Rate', xbins=100, xmin=-0.1, xmax=9.9)
        mbExpGroup.defineHistogram('MBTS_C_meanEnergy', type='TH1F',
                                   title='Mean MBTS Energy C side passed;MBTS Energy [pC];Entry Rate', xbins=100, xmin=-0.1, xmax=9.9)

        mbExpGroup.defineHistogram('MBTS_A_meanTime', type='TH1F',
                                   title='Mean MBTS Time A side; MBTS time [ns]', xbins=100, xmin=-100, xmax=100)
        mbExpGroup.defineHistogram('MBTS_C_meanTime', type='TH1F',
                                   title='Mean MBTS Time C side; MBTS time [ns]', xbins=100, xmin=-100, xmax=100)

    return monConfig.result()


if __name__ == '__main__':
    # Setup logs
    from AthenaCommon.Constants import DEBUG
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Output.HISTFileName = 'TestMBTSMonitorOutput.root'
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigMBTS(flags))

    cfg.getEventAlgo('HLTMBTSMonitoringAlgMT').OutputLevel = DEBUG  # DEBUG
    cfg.printConfig(withDetails=True)  # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()  # use cfg.run(20) to only run on first 20 events
    # to run:
    # python -m TrigMinBiasMonitoring.TrigMBTSMonitoringMT --filesInput=
