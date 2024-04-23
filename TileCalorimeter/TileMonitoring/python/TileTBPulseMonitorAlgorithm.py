#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTBPulseMonitorAlgorithm.py
@brief Python configuration of TileTBPulseMonitorAlgorithm algorithm for the Run III
'''

from AthenaConfiguration.Enums import Format
from TileMonitoring.TileMonitoringCfgHelper import getLegacyChannelForDemonstrator


def getPMT(partition, channel):
    ''' Function to get PMT number: 0,1 '''

    channelToPMT = [
        # LBA,LBC:
        0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,
        0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,
        0,  1,  0,  1,  0,  1, -1, -1,  0,  1,  0,  1,
        0,  1,  0,  1,  0,  1,  0, -1,  0,  1,  0,  1,
        # EBA,EBC:
        0,  0,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,
        0,  0,  0,  1,  0,  1, -1, -1,  0,  1,  0,  1,
        -1, -1, -1, -1, -1, -1,  0,  0,  1, -1, -1,  1,
        1,  1,  0,  0,  1,  0, -1, -1, -1, -1, -1, -1 ]

    # In gap scintillators (E3,E4,E1,E2) there is only one pmt per cell
    if partition in ['EBA', 'EBC'] and channel in [0, 1, 12, 13]:
        pmt = 0
    else:
        pmt = channelToPMT[channel+48] if partition in ['EBA', 'EBC'] else channelToPMT[channel]

        # Mirroring of odd/even numbers in negative side
        # (central symmetry of negative/positive drawers)
        if (pmt != -1 and partition in ['LBC', 'EBC']):
            pmt = 1 - pmt

    return pmt


def TileTBPulseMonitoringConfig(flags, timeRange=[-100, 100], fragIDs=[0x100, 0x101, 0x200, 0x201, 0x402], useDemoCabling=2018, **kwargs):

    ''' Function to configure TileTBPulseMonitorAlgorithm algorithm in the monitoring system.'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    result.merge(TileInfoLoaderCfg(flags))

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileTBPulseMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    tileTBPulseMonAlg = helper.addAlgorithm(CompFactory.TileTBPulseMonitorAlgorithm, 'TileTBPulseMonAlg')

    tileTBPulseMonAlg.TriggerChain = ''

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

    tileTBPulseMonAlg.TileFragIDs = fragIDs

    for k, v in kwargs.items():
        setattr(tileTBPulseMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])

    # Configure histogram with TileTBPulseMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileTBPulseMonAlg, 'TileTBPulseMonExecuteTime', 'TestBeam')
    executeTimeGroup.defineHistogram('TIME_execute', path='PulseShape', type='TH1F',
                                     title='Time for execute TileTBPulseMonAlg algorithm;time [#mus]',
                                     xbins=100, xmin=0, xmax=10000)

    from TileMonitoring.TileMonitoringCfgHelper import getCellName

    def addPulseShapeHistogramsArray(helper, modules, algorithm, name, title, path, type='TH2D',
                                     xbins=100, xmin=-100, xmax=100, ybins=100, ymin=-0.2, ymax=1.5,
                                     run='', value='', aliasPrefix='', useDemoCabling=2018):
        ''' This function configures 2D (or 1D Profile) histograms with Tile pulse shape per module, channel, gain  '''

        pulseShapeArray = helper.addArray([modules], algorithm, name, topPath=path)
        for postfix, tool in pulseShapeArray.Tools.items():
            moduleName = postfix[1:]
            partition = moduleName[:3]
            module = int(moduleName[3:]) - 1
            for channel in range(0, Tile.MAX_CHAN):
                legacyChannel = getLegacyChannelForDemonstrator(useDemoCabling, partition, module, channel)
                pmt = getPMT(partition, legacyChannel)
                pmtName = f'Channel_{channel}' if pmt < 0 else {0 : 'PMT_Up', 1 : 'PMT_Down'}[pmt]
                cell = getCellName(partition, legacyChannel)
                cellName = cell.replace('B', 'BC') if (partition in ['LBA','LBC'] and cell and cell[0] == 'B' and cell != 'B9') else cell
                for gain in range(0, Tile.MAX_GAIN):
                    gainName = {0 : 'lo', 1 : 'hi'}[gain]
                    fullPath = f'{partition}/{moduleName}'
                    name = f'time_{channel}_{gain},amplitude_{channel}_{gain};{aliasPrefix}{cellName}_{moduleName}_{pmtName}_{gainName}'
                    fullTitle = f'Run {run} {moduleName} Channel {channel} Gain {gainName}: {title};time [ns];Normalized Units'
                    tool.defineHistogram(name, title=fullTitle, path=fullPath, type=type,
                                         xbins=xbins, xmin=xmin, xmax=xmax, ybins=ybins, ymin=ymin, ymax=ymax)
        return pulseShapeArray

    addPulseShapeHistogramsArray(helper, modules, tileTBPulseMonAlg, name='TilePulseShape', title='Pulse shape',
                                 path='TestBeam/PulseShape', xbins=abs(timeRange[1]), xmin=timeRange[0], xmax=timeRange[1],
                                 run=run, aliasPrefix='pulseShape_', useDemoCabling=useDemoCabling)

    addPulseShapeHistogramsArray(helper, modules, tileTBPulseMonAlg, name='TilePulseShapeProfile',
                                 title='Pulse shape profile', path='TestBeam/PulseShape', type='TProfile',
                                 xbins=abs(timeRange[1]), xmin=timeRange[0], xmax=timeRange[1],
                                 ybins=None, ymin=None, ymax=None, run=run, aliasPrefix='pulseShapeProfile_',
                                 useDemoCabling=useDemoCabling)

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
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles

    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--digits', default="TileDigitsCnt", help='Tile digits container')
    parser.add_argument('--channels', default="TileRawChannelCnt",
                        help='Tile raw channel container, if empty they will be reconstructed from digits')
    parser.add_argument('--time-range', dest='timeRange', nargs=2, default=[-200, 200], help='Time range for pulse shape histograms')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201', '0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=15, help='Number of samples')
    parser.add_argument('--use-sqlite', dest='useSqlite', default='/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db',
                        help='Providing local SQlite file, conditions constants will be used from it')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]
    timeRange = [int(time) for time in args.timeRange]

    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Output.HISTFileName = 'TileTBPulseMonitorOutput.root'
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

    if args.channels:
        flags.Tile.RawChannelContainer = args.channels

    flags.fillFromArgs(parser=parser)
    flags.lock()

    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    rawChannels = args.channels
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
        cfg.merge( TileRawChannelMakerCfg(flags) )
        if args.threads and (args.threads > 1):
            rawChMaker = cfg.getEventAlgo('TileRChMaker')
            rawChMaker.Cardinality = args.threads

        if args.useSqlite:
            cfg.getService('IOVDbSvc').overrideTags += [
                f'<prefix>/TILE</prefix> <db>sqlite://;schema={args.useSqlite};dbname={flags.IOVDb.DatabaseInstance}</db>'
            ]

    cfg.merge(TileTBPulseMonitoringConfig(flags,
                                          fragIDs=fragIDs,
                                          timeRange=timeRange,
                                          useDemoCabling=args.demoCabling,
                                          TileRawChannelContainer=rawChannels,
                                          TileDigitsContainer=args.digits))

    tileInfoLoader = cfg.getService('TileInfoLoader')
    tileInfoLoader.NSamples = args.nsamples
    tileInfoLoader.TrigSample = (args.nsamples - 1) // 2  # Floor division

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store(open('TileTBPulseMonitorAlgorithm.pkl', 'wb'))

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
