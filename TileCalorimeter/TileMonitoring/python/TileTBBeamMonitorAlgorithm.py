#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTBBeamMonitorAlgorithm.py
@brief Python configuration of TileTBBeamMonitorAlgorithm algorithm for the Run III
'''

from AthenaConfiguration.Enums import Format


def TileTBBeamMonitoringConfig(flags, fragIDs=[0x100,0x101,0x200,0x201,0x402], **kwargs):

    ''' Function to configure TileTBBeamMonitorAlgorithm algorithm in the monitoring system.'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge(TileCablingSvcCfg(flags))

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileTBBeamMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    tileTBBeamMonAlg = helper.addAlgorithm(CompFactory.TileTBBeamMonitorAlgorithm, 'TileTBBeamMonAlg')

    tileTBBeamMonAlg.TriggerChain = ''

    kwargs.setdefault('CaloCellContainer', 'AllCalo')
    cellContainer = kwargs['CaloCellContainer']

    kwargs.setdefault('TBperiod', 2022)
    kwargs.setdefault('MaskMuonPMTs', [7])
    kwargs.setdefault('TOFDifferencePairs', [[2, 1], [2, 3], [3, 1]])

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

    tileTBBeamMonAlg.TileFragIDs = fragIDs

    for k, v in kwargs.items():
        setattr(tileTBBeamMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])

    # Configure histogram with TileTBBeamMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileTBBeamMonAlg, 'TileTBBeamMonExecuteTime', 'TestBeam')
    executeTimeGroup.defineHistogram('TIME_execute', path='BeamElements', type='TH1F',
                                     title='Time for execute TileTBBeamMonAlg algorithm;time [#mus]',
                                     xbins=100, xmin=0, xmax=10000)

    nMuonWallPMT = 12
    muonWallPMTArray = helper.addArray([nMuonWallPMT], tileTBBeamMonAlg, 'MuonWallPMT', topPath='TestBeam')
    for postfix, tool in muonWallPMTArray.Tools.items():
        pmt = int(postfix[1:]) + 1
        title = f'Run {run}: Muon Wall PMT{pmt} Amplitude;Amplitude [ADC];Counts'
        name = f'amplitude;MuonWallPMT{pmt}'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                             xbins=410, xmin=0, xmax=4096)

    totalMuEnergyGroup = helper.addGroup(tileTBBeamMonAlg, 'TileTBTotalMuonEnergy', 'TestBeam')
    totalMuEnergyGroup.defineHistogram('TotalMuonEnergy', path='BeamElements', type='TH1F',
                                       title=f'Run {run}: Muon Wall Total Energy; [ADC]',
                                       xbins=1500, xmin=0, xmax=10000)

    nScounters = 3
    sCountersArray = helper.addArray([nScounters], tileTBBeamMonAlg, 'Scounter', topPath='TestBeam')
    for postfix, tool in sCountersArray.Tools.items():
        counter = int(postfix[1:]) + 1
        title = f'Run {run}: S{counter} Counter Amplitude;Amplitude [ADC];Counts'
        name = f'amplitude;S{counter}hist'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                             xbins=410, xmin=0, xmax=4096)

    nCherenkov = 3
    cherenkovArray = helper.addArray([nCherenkov], tileTBBeamMonAlg, 'Cherenkov', topPath='TestBeam')
    for postfix, tool in cherenkovArray.Tools.items():
        cherenkov = int(postfix[1:]) + 1
        title = f'Run {run}: Cherenkov {cherenkov} Amplitude;Amplitude [ADC];Counts'
        name = f'amplitude;Cher{cherenkov}hist'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                             xbins=410, xmin=0, xmax=4096)


    cherCompGroup = helper.addGroup(tileTBBeamMonAlg, 'CherCompare', 'TestBeam')
    cherCompGroup.defineHistogram('amplitude1,amplitude2;CherCompare', path='BeamElements', type='TH2F',
                                  title=f'Run {run}: Cherenkov2 vs Cherenkov1;Amplitude [ADC];Amplitude [ADC]',
                                  xbins=410, xmin=0, xmax=4096, ybins=410, ymin=0, ymax=4096)

    nTOF = 3
    tofArray = helper.addArray([nTOF], tileTBBeamMonAlg, 'TOF', topPath='TestBeam')
    for postfix, tool in tofArray.Tools.items():
        tof = int(postfix[1:]) + 1
        title = f'Run {run}: TOF{tof};[ADC];Counts'
        name = f'amplitude;TOF{tof}'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                             xbins=4096, xmin=-0.5, xmax=4095.5)

    tofDiffPairs = kwargs['TOFDifferencePairs']
    tofDiffArray = helper.addArray([len(tofDiffPairs)], tileTBBeamMonAlg, 'TOFDiff', topPath='TestBeam')
    for postfix, tool in tofDiffArray.Tools.items():
        pairIdx = int(postfix.split('_').pop())
        tof1,tof2 = tofDiffPairs[pairIdx]
        title = f'Run {run}: TOF{tof1} - TOF{tof2} Amplitude difference;[ADC];Counts'
        tool.defineHistogram(f'TOFDiff;TOFDiff{tof1}{tof2}', title=title, path='BeamElements',
                             type='TH1F', xbins=4096, xmin=-0.5, xmax=4095.5)

    cherenkovVsTOFArray = helper.addArray([nCherenkov, nTOF], tileTBBeamMonAlg, 'CherenkovVsTOF', topPath='TestBeam')
    for postfix, tool in cherenkovVsTOFArray.Tools.items():
        cherenkovTof = postfix.split('_')
        tof = int(cherenkovTof.pop()) + 1
        cherenkov = int(cherenkovTof.pop()) + 1
        title = f'Run {run}: Cherenkov {cherenkov} Amplitude vs TOF{tof} Amplitude'
        title += f';TOF{tof} Amplitude [ADC];Cherenkov{cherenkov} Amplitude [ADC]'
        name = f'amplitudeTOF,amplitudeCherenkov;Cher{cherenkov}TOF{tof}'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH2F',
                             xbins=410, xmin=0, xmax=4096, ybins=410, ymin=0, ymax=4096)

    pmtHitMapGroup = helper.addGroup(tileTBBeamMonAlg, 'PMTHitMap', 'TestBeam')
    pmtHitMapGroup.defineHistogram('column,row,amplitude;PMTHitMap', path='BeamElements', type='TProfile2D',
                                   title=f'Run {run}: Muon Wall PMT Hit Map',
                                   xbins=4, xmin=0, xmax=4, ybins=2, ymin=0, ymax=2)

    nScaler = 3
    scalerArray = helper.addArray([nScaler], tileTBBeamMonAlg, 'Scaler', topPath='TestBeam')
    for postfix, tool in scalerArray.Tools.items():
        scaler = int(postfix[1:]) + 1
        title = f'Run {run}: Scaler S{scaler};Counts;# Events'
        name = f'counts;Scaler{scaler}'
        tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                             xbins=20000, xmin=-0.5, xmax=19999.5)

    scalerCoincedenceGroup = helper.addGroup(tileTBBeamMonAlg, 'Scaler12', 'TestBeam')
    scalerCoincedenceGroup.defineHistogram('counts12;Scaler12', path='BeamElements', type='TH1F',
                                           title=f'Run {run}: Scaler S1 and S2 coincedence;counts;# Events',
                                           xbins=20000, xmin=-0.5, xmax=19999.5)


    beamChambers = ['BC1', 'BC2']
    beamChambersArray = helper.addArray([beamChambers], tileTBBeamMonAlg, 'BeamChamber', topPath='TestBeam')
    for postfix, tool in beamChambersArray.Tools.items():
        beamChamber = postfix[1:]

        for coordinate in ['X', 'Y']:
            title = f'Run {run}: {beamChamber}{coordinate} Coordinate;{coordinate}[mm];Counts'
            name = f'{beamChamber}{coordinate};{beamChamber}{coordinate}hist'
            tool.defineHistogram(name, title=title, path='BeamElements', type='TH1F',
                                 xbins=201, xmin=-100.5, xmax=100.5)

        tool.defineHistogram(f'{beamChamber}X,{beamChamber}Y;{beamChamber}Profile', path='BeamElements',
                             type='TH2D', title=f'Run {run}: {beamChamber} Beam Profile;X[mm];Y[mm]',
                             xbins=1000, xmin=-100, xmax=100, ybins=1000, ymin=-100, ymax=100)

        tool.defineHistogram(f'{beamChamber}Xsum,{beamChamber}Ysum;{beamChamber}ProfileSum', path='BeamElements',
                             type='TH2D', title=f'Run {run}: {beamChamber} Beam Profile Sum;X [mm];Y [mm]',
                             xbins=1000, xmin=-300, xmax=0, ybins=1000, ymin=-300, ymax=0)


    impactProfileGroup = helper.addGroup(tileTBBeamMonAlg, 'ImpactProfile', 'TestBeam')
    impactProfileGroup.defineHistogram('Ximp,Yimp;ImpactProfile', path='BeamElements', type='TH2F',
                                       title=f'Run {run}: {beamChamber} Impact Profile;X [mm];Y [mm]',
                                       xbins=200, xmin=-100, xmax=100, ybins=200, ymin=-100, ymax=100)


    if cellContainer:
        cherenkovEnergyArray = helper.addArray([nCherenkov], tileTBBeamMonAlg, 'CherenkovVsEnergy', topPath='TestBeam')
        for postfix, tool in cherenkovEnergyArray.Tools.items():
            cherenkov = int(postfix[1:]) + 1
            title = f'Run {run}: Cherenkov {cherenkov} Amplitude vs Total Energy;Energy [pC]; Amplitude [ADC]'
            name = f'totalEnergy,amplitude;Cher{cherenkov}Energy'
            tool.defineHistogram(name, title=title, path='BeamElements', type='TH2F',
                                 xbins=150, xmin=0, xmax=150, ybins=410, ymin=0, ymax=4096)

        cellEneXimpGroup = helper.addGroup(tileTBBeamMonAlg, 'CellEnergyImpactX', 'TestBeam')
        cellEneXimpGroup.defineHistogram('Ximp,cellEnergy;CellEnergyImpactX', path='BeamElements', type='TH2F',
                                         title=f'Run {run}:  Maximum Cell Energy vs Impact X;X [mm];Cell Energy [pc]',
                                         xbins=200, xmin=-100, xmax=100, ybins=150, ymin=0, ymax=150)

        cellEneYimpGroup = helper.addGroup(tileTBBeamMonAlg, 'CellEnergyImpactY', 'TestBeam')
        cellEneYimpGroup.defineHistogram('Yimp,cellEnergy;CellEnergyImpactY', path='BeamElements', type='TH2F',
                                         title=f'Run {run}:  Maximum Cell Energy vs Impact Y;Y [mm];Cell Energy [pc]',
                                         xbins=200, xmin=-100, xmax=100, ybins=150, ymin=0, ymax=150)

        totalEneXimpGroup = helper.addGroup(tileTBBeamMonAlg, 'TotalEnergyImpactX', 'TestBeam')
        totalEneXimpGroup.defineHistogram('Ximp,totalEnergy;TotalEnergyImpactX', path='BeamElements', type='TH2F',
                                          title=f'Run {run}: Total Energy vs Impact X;X [mm];Total Energy [pc]',
                                          xbins=200, xmin=-100, xmax=100, ybins=150, ymin=0, ymax=150)

        totalEneYimpGroup = helper.addGroup(tileTBBeamMonAlg, 'TotalEnergyImpactY', 'TestBeam')
        totalEneYimpGroup.defineHistogram('Yimp,totalEnergy;TotalEnergyImpactY', path='BeamElements', type='TH2F',
                                          title=f'Run {run}: Total Energy vs Impact Y;Y [mm];Total Energy [pc]',
                                          xbins=200, xmin=-100, xmax=100, ybins=150, ymin=0, ymax=150)


        totalEneS1Group = helper.addGroup(tileTBBeamMonAlg, 'ScinCalEnergy', 'TestBeam')
        totalEneS1Group.defineHistogram('amplitude,totalEnergy;ScinCalEnergy', path='BeamElements', type='TH2F',
                                        title=f'Run {run}: Total Energy vs S1 amplitude; Amplitude [ADC];Total Energy [pc]',
                                        xbins=410, xmin=0, xmax=4096, ybins=150, ymin=0, ymax=150)


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
    parser.add_argument('--digits', default="TileDigitsCnt", help='Tile digits container')
    parser.add_argument('--channels', default="TileRawChannelCnt",
                        help='Tile raw channel container, if empty they will be reconstructed from digits')
    parser.add_argument('--cells', default="AllCalo",
                        help='Calo Cells container, if empty they will be reconstructed from channels')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201','0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=15, help='Number of samples')
    parser.add_argument('--use-sqlite', dest='useSqlite', default='/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db',
                        help='Providing local SQlite file, conditions constants will be used from it')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Output.HISTFileName = 'TileTBBeamMonitorOutput.root'
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


    cfg.merge(TileTBBeamMonitoringConfig(flags, TBperiod=2021, fragIDs=fragIDs))

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

    cfg.store(open('TileTBBeamMonitorAlgorithm.pkl', 'wb'))

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
