#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTBMonitorAlgorithm.py
@brief Python configuration of TileTBMonitorAlgorithm algorithm for the Run III
'''

from AthenaConfiguration.Enums import Format
from AthenaCommon.SystemOfUnits import GeV

def TileTBMonitoringConfig(flags, fragIDs=[0x100, 0x101, 0x200, 0x201, 0x402], **kwargs):

    ''' Function to configure TileTBMonitorAlgorithm algorithm in the monitoring system.'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge(TileCablingSvcCfg(flags))

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileTBMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    tileTBMonAlg = helper.addAlgorithm(CompFactory.TileTBMonitorAlgorithm, 'TileTBMonAlg')

    tileTBMonAlg.TriggerChain = ''

    kwargs.setdefault('BeamEnergy', flags.Beam.Energy)
    kwargs.setdefault('CellEnergyThreshold', 0.1 * GeV)
    energyThreshold = kwargs['CellEnergyThreshold']

    masked = ['LBA01 0 ' + ",".join([str(channel) for channel in range(0, 48)]),
              'LBA01 1 ' + ",".join([str(channel) for channel in range(0, 48)]),
              'LBC01 0 36,37,38,39,40,41',
              'LBC01 1 36,37,38,39,40,41']
    kwargs.setdefault('Masked', masked)

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

    tileTBMonAlg.TileFragIDs = fragIDs

    for k, v in kwargs.items():
        setattr(tileTBMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])

    # Configure histogram with TileTBMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileTBMonAlg, 'TileTBMonExecuteTime', 'TestBeam')
    executeTimeGroup.defineHistogram('TIME_execute', path='', type='TH1F',
                                     title='Time for execute TileTBMonAlg algorithm;time [#mus]',
                                     xbins=100, xmin=0, xmax=10000)

    totalEnergyGroup = helper.addGroup(tileTBMonAlg, 'TileTBTotalEventEnergy', 'TestBeam')
    totalEnergyGroup.defineHistogram('energy;TileTBTotalEventEnergy', path='', type='TH1F',
                                     title=f'Run {run}: Total TileCal Event Energy;Event Energy [pC]',
                                     xbins=400, xmin=-2, xmax=200)

    hotCellAGroup = helper.addGroup(tileTBMonAlg, 'TileTBHotCellA_LBC02', 'TestBeam')
    hotCellAGroup.defineHistogram('tower;TileTBHotCellA_LBC02', path='', type='TH1F',
                                  title=f'Run {run} LBC02: Tile TB Hot Cell A;Tower',
                                  xbins=10, xmin=-0.5, xmax=9.5)

    CtotGroup = helper.addGroup(tileTBMonAlg, 'TileTBCtot', 'TestBeam')
    CtotGroup.defineHistogram('Ctot;TileTBCtot', path='', type='TH1F',
                              title=f'Run {run} LBC02: Tile TB Ctot;Ctot',
                              xbins=100, xmin=0.03, xmax=0.22)

    ClongGroup = helper.addGroup(tileTBMonAlg, 'TileTBClong', 'TestBeam')
    ClongGroup.defineHistogram('Clong;TileTBClong', path='', type='TH1F',
                               title=f'Run {run} LBC02: Tile TB Clong;Clong',
                               xbins=100, xmin=0., xmax=1.8)

    CtotVsClongGroup = helper.addGroup(tileTBMonAlg, 'TileTBCtotVsClong', 'TestBeam')
    CtotVsClongGroup.defineHistogram('Clong,Ctot;TileTBCtotVsClong', path='', type='TH2F',
                                     title=f'Run {run} LBC02: Tile TB Ctot Vs Clong;Clong;Ctot',
                                     xbins=100, xmin=0., xmax=1.8, ybins=100, ymin=0.03, ymax=0.22)


    maxTotalEnergy = 150
    nCellsVsEnergyGroup = helper.addGroup(tileTBMonAlg, 'TileTBCellsNumberVsTotalEnergy', 'TestBeam')
    nCellsVsEnergyGroup.defineHistogram('nCells,energy;TileTBCellsNumberVsTotalEnergy', path='', type='TH2F',
                                        title=f'Run {run}: Tile Event energy [C side] vs # cells with energy > {energyThreshold} pC;# Cells;Energy [pC]',
                                        xbins=25, xmin=-0.5, xmax=24.5, ybins=maxTotalEnergy, ymin=0.0, ymax=maxTotalEnergy)

    hitMapGroup = helper.addGroup(tileTBMonAlg, 'TileTBHitMap', 'TestBeam')
    hitMapGroup.defineHistogram('side,module,energy;TileTBHitMap', path='', type='TProfile2D',
                                title=f'Run {run}: Tile TB setup map with average energy',
                                xlabels=['A Side', 'C side'], ylabels=['LB01', 'LB02', 'EB03'],
                                xbins=2, xmin=-0.5, xmax=1.5, ybins=3, ymin=-0.5, ymax=2.5)

    timeArray = helper.addArray([modules], tileTBMonAlg, 'TileTBChannelTime', topPath='TestBeam')
    for postfix, tool in timeArray.Tools.items():
        moduleName = postfix[1:]
        name = f'channel,time;TileTBChannelTime_{moduleName}'
        fullTitle = f'Run {run} {moduleName}: Tile TB channel average time from cells;Channel;Time [ns]'
        tool.defineHistogram(name, title=fullTitle, path='', type='TProfile',
                             xbins=Tile.MAX_CHAN, xmin=-0.5, xmax=Tile.MAX_CHAN-0.5)

    maxSample = 3
    maxTowerLB = 10
    xCellLB = [[[] for tower in range(0, maxTowerLB)] for sample in range(0, maxSample)]
    yCellLB = [[[] for tower in range(0, maxTowerLB)] for sample in range(0, maxSample)]

    periodWidthLB = 18.22
    # Approximate number of periods in Tile Cells in LB per sampling and tower
    nPeriodsLB = [[14, 13, 14, 14, 15, 16, 16, 17, 19, 16],  # A1-A10
                  [16, 15, 16, 16, 17, 18, 18, 20, 18],      # B1-B9
                  [18, 18, 18, 19, 19, 20, 22, 20],          # C1-C8
                  [20, 0, 41, 0, 43, 0, 50]]                 # D0-D3

    yLB = [0, 300, 690, 1140, 1520]
    for sampleIndex in range(0, len(nPeriodsLB)):
        sample = sampleIndex if sampleIndex < 2 else sampleIndex - 1
        cellOffsetX = 0.0
        for tower in range(0, len(nPeriodsLB[sampleIndex])):
            cellWidth = nPeriodsLB[sampleIndex][tower] * periodWidthLB
            x1 = cellOffsetX + 9  # Approximate center of the period
            x2 = cellOffsetX + cellWidth
            while x1 < x2:
                xCellLB[sample][tower] += [x1]
                yCellLB[sample][tower] += [yLB[sampleIndex]]
                x1 += periodWidthLB

            cellOffsetX += cellWidth

    tileTBMonAlg.xCellLongBarrelSampleA = xCellLB[0]
    tileTBMonAlg.xCellLongBarrelSampleBC = xCellLB[1]
    tileTBMonAlg.xCellLongBarrelSampleD = xCellLB[2]

    tileTBMonAlg.yCellLongBarrelSampleA = yCellLB[0]
    tileTBMonAlg.yCellLongBarrelSampleBC = yCellLB[1]
    tileTBMonAlg.yCellLongBarrelSampleD = yCellLB[2]

    maxTowerEB = 16
    periodWidthEB = 18.28
    xCellEB = [[[] for tower in range(0, maxTowerEB)] for sample in range(0, maxSample)]
    yCellEB = [[[] for tower in range(0, maxTowerEB)] for sample in range(0, maxSample)]

    yEB = [0, 300, 690, 840, 840, 1140, 1520]
    # Approximate number of periods in Tile Cells in EB per sampling and tower
    nPeriodsEB = [[17, 0,  0,  9, 25, 28, 30, 48],  # A12-A16
                  [17, 0, 16, 27, 30, 32, 35],      # B11-B15
                  [12, 5, 16, 27, 30, 32, 35],      # C10, B11-B15
                  [12, 5],                          # C10
                  [0,  17, 65, 0, 75],              # D5,D6
                  [17,  0, 65, 0, 75]]              # D4,D5,D6

    samples = [0, 1, 1, 1, 2, 2]
    for sampleIndex in range(0, len(nPeriodsEB)):
        sample = samples[sampleIndex]
        cellOffsetX = 0.0
        for tower in range(0, len(nPeriodsEB[sampleIndex])):
            cellWidth = nPeriodsEB[sampleIndex][tower] * periodWidthEB
            x1 = cellOffsetX + 9 # Approximate center of the period
            x2 = cellOffsetX + cellWidth
            while x1 < x2:
                xCellEB[sample][tower + 8] += [x1]
                yCellEB[sample][tower + 8] += [yEB[sampleIndex]]
                x1 += periodWidthEB

            cellOffsetX += cellWidth

    tileTBMonAlg.xCellExtendedBarrelSampleA = xCellEB[0]
    tileTBMonAlg.xCellExtendedBarrelSampleBC = xCellEB[1]
    tileTBMonAlg.xCellExtendedBarrelSampleD = xCellEB[2]

    tileTBMonAlg.yCellExtendedBarrelSampleA = yCellEB[0]
    tileTBMonAlg.yCellExtendedBarrelSampleBC = yCellEB[1]
    tileTBMonAlg.yCellExtendedBarrelSampleD = yCellEB[2]

    xBinsLB = [x * periodWidthLB for x in range(0, sum(nPeriodsLB[0]) + 1)]
    yBinsLB = [0, 300, 690, 1140, 1520]

    xBinsEB = [x * periodWidthEB for x in range(0, sum(nPeriodsEB[0]) + 1)]
    yBinsEB = [0, 300, 690, 840, 1140, 1520]
    cellMapArray = helper.addArray([modules], tileTBMonAlg, 'TileTBCellMap', topPath='TestBeam')
    for postfix, tool in cellMapArray.Tools.items():
        moduleName = postfix[1:]
        name = f'x,y,energy;TileTBCellMap_{moduleName}'
        fullTitle = f'Run {run} {moduleName}: Tile TB cell map with average energy;z [mm];x [mm];Energy [pc]'
        tool.defineHistogram(name, title=fullTitle, path='', type='TProfile2D',
                             xbins=xBinsLB if moduleName.startswith('LB') else xBinsEB,
                             ybins=yBinsLB if moduleName.startswith('LB') else yBinsEB)

    accumalator = helper.result()
    result.merge(accumalator)
    return result


if __name__ == '__main__':

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
    parser.add_argument('--digits', default="TileDigitsCnt", help='Tile digits container')
    parser.add_argument('--channels', default="TileRawChannelCnt",
                        help='Tile raw channel container, if empty they will be reconstructed from digits')
    parser.add_argument('--cells', default="AllCalo",
                        help='Calo Cells container, if empty they will be reconstructed from channels')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201', '0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=15, help='Number of samples')
    parser.add_argument('--use-sqlite', dest='useSqlite', default='/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db',
                        help='Providing local SQlite file, conditions constants will be used from it')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    flags.Input.Files = defaultTestFiles.ESD
    flags.Output.HISTFileName = 'TileTBMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.Common.isOnline = True

    flags.Tile.doFit = True
    flags.Tile.useDCS = False
    flags.Tile.NoiseFilter = 0
    flags.Tile.correctTime = False
    flags.Tile.correctTimeJumps = False
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.doOverflowFit = False


    flags.fillFromArgs(parser=parser)
    flags.lock()

    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    rawChannels = args.channels
    cells = args.cells
    if flags.Input.Format is Format.BS:
        readDigitsFlx = 'Flx' in args.digits
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                         readDigits=(not readDigitsFlx),
                                         readDigitsFlx=readDigitsFlx) )

    else:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    if not rawChannels:
        # Run reconstruction to produce Tile raw channels
        rawChannels = flags.Tile.RawChannelContainer

        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        cfg.merge(TileRawChannelMakerCfg(flags))
        if args.threads and (args.threads > 1):
            rawChMaker = cfg.getEventAlgo('TileRChMaker')
            rawChMaker.Cardinality = args.threads

        if args.useSqlite:
            cfg.getService('IOVDbSvc').overrideTags += [
                f'<prefix>/TILE</prefix> <db>sqlite://;schema={args.useSqlite};dbname={flags.IOVDb.DatabaseInstance}</db>'
            ]

    if not cells:
        # Run reconstruction to produce Tile cells
        cells = 'AllCalo'
        from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
        cfg.merge(TileCellMakerCfg(flags, maskBadChannels=False, mergeChannels=False, UseDemoCabling=args.demoCabling))

    cfg.merge(TileTBMonitoringConfig(flags, fragIDs=fragIDs, CaloCellContainer=cells))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    cfg.merge(TileInfoLoaderCfg(flags))
    tileInfoLoader = cfg.getService('TileInfoLoader')
    tileInfoLoader.NSamples = args.nsamples
    tileInfoLoader.TrigSample = (args.nsamples - 1) // 2  # Floor division

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store(open('TileTBMonitorAlgorithm.pkl', 'wb'))

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
