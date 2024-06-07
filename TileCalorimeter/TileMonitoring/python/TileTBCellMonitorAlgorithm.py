#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTBCellMonitorAlgorithm.py
@brief Python configuration of TileTBCellMonitorAlgorithm algorithm for the Run III
'''

from AthenaCommon.SystemOfUnits import GeV


def TileTBCellMonitoringConfig(flags, timeRange=[-100, 100], fragIDs=[0x100, 0x101, 0x200, 0x201, 0x402], useDemoCabling=2018, useFELIX=False, **kwargs):

    ''' Function to configure TileTBCellMonitorAlgorithm algorithm in the monitoring system.'''

    suffix = "Flx" if useFELIX else ""
    basePath = 'TestBeam/' + ('Felix' if useFELIX else 'Legacy') + '/Cell'

    cellContainer = kwargs.pop('CaloCellContainer',  f'AllCalo{suffix}')
    kwargs.setdefault('ScaleFactor', 0.25 if useFELIX else 1.0)
    kwargs.setdefault('EnergyThresholdForTime', 1 * GeV)

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    result.merge(TileInfoLoaderCfg(flags))

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, f'TileTBCell{suffix}Monitoring')

    demoCabling = kwargs.pop('useDemoCabling', 2018)
    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    modules = []
    if fragIDs:
        for fragID in fragIDs:
            ros = fragID >> 8
            drawer = fragID & 0x3F
            modules += [Tile.getDrawerString(ros, drawer)]
    else:
        for ros in range(1, Tile.MAX_ROS):
            for drawer in range(0, Tile.MAX_DRAWER):
                fragIDs += [(ros << 8) | drawer]
                modules += [Tile.getDrawerString(ros, drawer)]

    cellMonAlgorithms = []
    gains = ['HG', 'LG'] if flags.Tile.RunType.isBiGain() else [""]
    from AthenaConfiguration.ComponentFactory import CompFactory
    for gain in gains:
        cellMonAlg = helper.addAlgorithm(CompFactory.TileTBCellMonitorAlgorithm, f'TileTBCell{suffix}{gain}MonAlg')
        cellMonAlg.CaloCellContainer = f'{cellContainer}{gain}'
        cellMonAlg.TriggerChain = ''
        cellMonAlg.TileFragIDs = fragIDs

        for k, v in kwargs.items():
            setattr(cellMonAlg, k, v)

        cellMonAlgorithms += [cellMonAlg]

    towersLB = [[tower for tower in range(0, 10)],   # Sample A
                [tower for tower in range(0, 9)],    # Sample BC/B
                [tower*2 for tower in range(0, 4)]]  # Sample D

    towersEB = [[tower for tower in range(11, 16)],  # Sample A
                [tower for tower in range(9, 15)],   # Sample B/C
                [tower*2 for tower in range(4, 7)]]  # Sample D

    def getCellNameFromSampleAndTower(sample, tower):
        ''' The function to get Tile cell name from sample and tower'''
        sampleName = {0: 'A', 1: 'B', 2: 'D'}[sample]
        if sample == 1:
            if tower < 8:
                sampleName += 'C'  # BC1 ... BC8 in LB
            elif tower == 9:
                sampleName = 'C'   # C10 in EB
        cellName = f'{sampleName}{tower + 1}' if sample < 2 else f'{sampleName}{int(tower / 2)}'
        return cellName

    def addCellHistogramsArray(helper, modules, algorithm, name, title, path='', type='TH1D',
                               xbins=100, xmin=-100, xmax=100, ybins=None, ymin=None, ymax=None,
                               run='', xvalue='', yvalue=None, aliasPrefix='', xtitle='', ytitle=''):
        ''' This function configures 1D or 2D histograms with monitored value per Tile module and cell '''

        cellArray = helper.addArray([modules], algorithm, name, topPath=path)
        for postfix, tool in cellArray.Tools.items():
            moduleName = postfix[1:]
            partition = moduleName[:3]
            towers = towersLB if moduleName.startswith('L') else towersEB
            for sample in range(0, 3):
                for tower in towers[sample]:
                    cellName = getCellNameFromSampleAndTower(sample, tower)
                    fullPath = f'{partition}/{moduleName}'
                    name = f'{xvalue}_{sample}_{tower}'
                    if yvalue:
                        name += f',{yvalue}_{sample}_{tower}'
                    name += f';{aliasPrefix}{cellName}_{moduleName}'
                    fullTitle = f'Run {run} {moduleName} {cellName}: {title};{xtitle};{ytitle}'
                    tool.defineHistogram(name, title=fullTitle, path=fullPath, type=type,
                                         xbins=xbins, xmin=xmin, xmax=xmax,
                                         ybins=ybins, ymin=ymin, ymax=ymax)
        return cellArray

    from TileMonitoring.TileMonitoringCfgHelper import getCellName
    from TileMonitoring.TileMonitoringCfgHelper import getLegacyChannelForDemonstrator

    def addChannelHistogramsArray(helper, modules, algorithm, name, title, path='', type='TH1D',
                                  xbins=100, xmin=-100, xmax=100, xvalue='', xtitle='', ytitle='',
                                  run='', aliasPrefix='', useDemoCabling=demoCabling):
        ''' This function configures 1D histograms with Tile monitored value per module and channel '''

        channelArray = helper.addArray([modules], algorithm, name, topPath=path)
        for postfix, tool in channelArray.Tools.items():
            moduleName = postfix[1:]
            partition = moduleName[:3]
            module = int(moduleName[3:]) - 1
            for channel in range(0, Tile.MAX_CHAN):
                legacyChannel = getLegacyChannelForDemonstrator(useDemoCabling, partition, module, channel)
                cell = getCellName(partition, legacyChannel)
                if cell:
                    cellName = cell.replace('B', 'BC') if (partition in ['LBA','LBC'] and cell and cell[0] == 'B' and cell != 'B9') else cell
                    fullPath = f'{partition}/{moduleName}'
                    name = f'{xvalue}_{channel};{aliasPrefix}_{moduleName}_{cellName}_ch_{channel}'
                    fullTitle = f'Run {run} {moduleName} {cellName} Channel {channel}: {title};{xtitle};{ytitle}'
                    tool.defineHistogram(name, title=fullTitle, path=fullPath, type=type,
                                         xbins=xbins, xmin=xmin, xmax=xmax)
        return channelArray

    totalEnergy = min(flags.Beam.Energy / GeV, 300)
    nEnergyBins = int(totalEnergy * 2)
    run = str(flags.Input.RunNumbers[0])
    nTimeBins = timeRange[1] - timeRange[0]

    for cellMonAlg in cellMonAlgorithms:
        topPath = f'{basePath}/LG' if 'LG' in cellMonAlg.name else basePath

        # Configure histogram with TileTBCellMonAlg algorithm execution time
        executeTimeGroup = helper.addGroup(cellMonAlg, 'TileTBCellMonExecuteTime', topPath)
        executeTimeGroup.defineHistogram('TIME_execute', path='', type='TH1F',
                                         title=f'Time for execute TileTBCell{suffix}MonAlg algorithm;time [#mus]',
                                         xbins=100, xmin=0, xmax=10000)

        sampleEnergyArray = helper.addArray([modules], cellMonAlg, 'TileSampleEnergy', topPath=topPath)
        for postfix, tool in sampleEnergyArray.Tools.items():
            moduleName = postfix[1:]
            partition = moduleName[:3]
            fullPath = f'{partition}/{moduleName}'
            titlePrefix = f'Run {run} {moduleName}:'

            tool.defineHistogram(f'energy;EnergyTotal_{moduleName}', path=fullPath, type='TH1D',
                                 title=f'{titlePrefix} Total energy;Energy [pC];Entries',
                                 xbins=nEnergyBins, xmin=0.0, xmax=totalEnergy)

            tool.defineHistogram(f'energyA,energyBC;EnergyTotalSampleBCVsA_{moduleName}', path=fullPath, type='TH2D',
                                 title=f'{titlePrefix} Total energy in sample BC vs sample A;Sample A Energy [pC];Sample B Energy [pC]',
                                 xbins=nEnergyBins, xmin=0.0, xmax=totalEnergy, ybins=nEnergyBins, ymin=0.0, ymax=totalEnergy)

            tool.defineHistogram(f'energyD;EnergyTotalSampleD_{moduleName}', path=fullPath, type='TH1D',
                                 title=f'{titlePrefix} Total energy in sample D;Sample D Energy [pC];Entries',
                                 xbins=nEnergyBins, xmin=0.0, xmax=totalEnergy)

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellEnergy', path=topPath, xvalue='energy',
                               title='Tile Cell Energy', xbins=nEnergyBins, xmin=0, xmax=totalEnergy,
                               run=run, aliasPrefix='CellEnergy', xtitle='Energy [pC]', ytitle='Entries')

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellEnergyDiff', path=topPath, xvalue='energyDiff',
                               title='Tile Cell Energy difference between PMTs', xbins=nEnergyBins, xmin=-totalEnergy, xmax=totalEnergy,
                               run=run, aliasPrefix='CellEnergyDiff', xtitle='Energy [pC]', ytitle='Entries')

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellTime', path=topPath, xvalue='time',
                               title='Tile Cell Time', xbins=nTimeBins, xmin=timeRange[0], xmax=timeRange[1],
                               run=run, aliasPrefix='CellTime', xtitle='Time [ns]', ytitle='Entries')

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellTimeDiff', path=topPath,
                               xvalue='timeDiff', title='Tile Cell Time difference between PMTs',
                               xbins=nTimeBins, xmin=timeRange[0], xmax=timeRange[1],
                               run=run, aliasPrefix='CellTimeDiff', xtitle='Time [ns]', ytitle='Entries')

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellEnergyLeftVsRightPMT', path=topPath, type='TH2D',
                               xvalue='energy1', yvalue='energy2', title='Tile Cell PMT2 vs PMT1 Energy',
                               xbins=nEnergyBins, xmin=0, xmax=totalEnergy, ybins=nEnergyBins, ymin=0, ymax=totalEnergy,
                               run=run, aliasPrefix='CellEnergyLeftVsRightPMT',
                               xtitle='Energy [pC]', ytitle='Energy [pC]')

        addCellHistogramsArray(helper, modules, cellMonAlg, name='TileCellTimeLeftVsRightPMT', path=topPath, type='TH2D',
                               xvalue='time1', yvalue='time2', title='Tile Cell PMT2 vs PMT1 Time',
                               xbins=nTimeBins, xmin=timeRange[0], xmax=timeRange[1], ybins=nTimeBins, ymin=timeRange[0], ymax=timeRange[1],
                               run=run, aliasPrefix='CellTimeLeftVsRightPMT', xtitle='Time [ns]', ytitle='Time [ns]')

        addChannelHistogramsArray(helper, modules, cellMonAlg, name='TileChannelEnergy', path=f'{topPath}/ChannelEnergy', type='TH1D',
                                  xvalue='energy', title='Tile channel energy', xbins=nEnergyBins, xmin=0, xmax=totalEnergy,
                                  run=run, aliasPrefix='ChannelEnergy', xtitle='Energy [pC]', ytitle='Entries')

        addChannelHistogramsArray(helper, modules, cellMonAlg, name='TileChannelTime', path=f'{topPath}/ChannelTime', type='TH1D',
                                  xvalue='time', title='Tile channel time', xbins=nTimeBins, xmin=timeRange[0], xmax=timeRange[1],
                                  run=run, aliasPrefix='ChannelTime', xtitle='Time [ns]', ytitle='Entries')

    accumalator = helper.result()
    result.merge(accumalator)
    return result


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--time-range', dest='timeRange', nargs=2, default=[-200, 200], help='Time range for pulse shape histograms')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201', '0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]
    timeRange = [int(time) for time in args.timeRange]

    flags.Input.Files = defaultTestFiles.ESD
    flags.Output.HISTFileName = 'TileTBCellMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.Common.isOnline = True

    flags.fillFromArgs(parser=parser)
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(TileTBCellMonitoringConfig(flags,
                                         fragIDs=fragIDs,
                                         timeRange=timeRange,
                                         useDemoCabling=args.demoCabling))

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store(open('TileTBCellMonitorAlgorithm.pkl', 'wb'))

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
