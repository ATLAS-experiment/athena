#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''@file RunTileCalibRec.py
@brief Script to run Tile Reconstrcution/Monitoring for calibration runs
'''

from TileRecEx import TileInputFiles
from AthenaConfiguration.Enums import Format
from TileConfiguration.TileConfigFlags import TileRunType

epiLog = """
Examples:

    RunTileCalibRec.py --run RUNNUMBER --evtMax 1 --pedestals --loglevel WARNING IOVDb.GlobalTag='CONDBR2-BLKPA-2018-13'
    RunTileCalibRec.py --filesInput=FILE1,FILE2 Tile.RunType=TileRunType.LAS Tile.correctTime=True Exec.SkipEvents=100
    RunTileCalibRec.py --run RUNNUMBER --cis --postExec='useTagFor("RUN2-HLT-UPD1-00", "/TILE/OFL02/TIME/CHANNELOFFSET/CIS");'
    RunTileCalibRec.py --run RUNNUMBER --laser --postExec='useSqliteFor("tileSqlite.db", "/TILE");'

    RunTileCalibRec.py --run RUNNUMBER @options.txt

        where "options.txt" is text file which can contain all arguments and Athen configuration flags (one per line), e.g.:
          --run2
          --upd4
          --laser
          --loglevel=INFO
          --postExec=useSqliteAndTagFor("tileSqlite.db", "RUN2-HLT-UPD1-00", "/TILE/OFL02/TIME/CHANNELOFFSET/LAS");
          Tile.correctTime=True
          Exec.SkipEvents=100

At least one should provide the following arguments or Athena configuration flags (flags have higher priority):
   Input file(s), e.g.: --run RUNNUMBER | --filesInput=FILE1,FILE2 | Input.Files="['FILE1','FILE2']"
   Tile Run Type, e.g.: --cis | --laser | --pedestals | --physics | Tile.RunType=TileRunType.PHY
"""


def getArgumentParser(flags):
    """ Function to return command line arguments parser for reconstuction of Tile calibration runs """

    parserParents = [flags.getArgumentParser(), TileInputFiles.getArgumentParser(add_help=False)]

    import argparse
    parser= argparse.ArgumentParser(parents=parserParents, add_help=False, fromfile_prefix_chars='@', epilog=epiLog, formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--printDetailedConfig', action='store_true', help='Print detailed Athena configuration')
    parser.add_argument('--dumpArguments', action='store_true', help='Print arguments and exit')
    parser.add_argument('--outputDirectory', default='.', help='Output directory for produced files')

    parser.add_argument('--perfmon', action='store_true', help='Run perfmon')

    parser.add_argument('-v', '--version', type=str, default='0', help='Version to be used in output files for ntuple and monitoring')

    parser.add_argument('--calib', default=False, help='Calculate calibration constants and store them in ROOT file', action=argparse.BooleanOptionalAction)
    parser.add_argument('--tmdb', default=None, help='Enable TMDB', action=argparse.BooleanOptionalAction)

    # Set up Tile h2000 ntuple
    ntuple = parser.add_argument_group('Tile h2000 ntuple')
    ntuple.add_argument('--ntuple', default=True, help='Create Tile h2000 ntuple', action=argparse.BooleanOptionalAction)
    ntuple.add_argument('--reduced-ntuple', dest='reduced_ntuple', action='store_true', help='No Tile raw cahnnel container (including DSP) in h2000 ntuple')

    # Set up Tile monitoring
    mon = parser.add_argument_group('Tile monitoring')
    mon.add_argument('--mon', default=False, help='Run Tile monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--postprocessing', default=True, help='Run Tile monitoring postprocessing', action=argparse.BooleanOptionalAction)
    mon.add_argument('--digits-mon', dest='digits_mon', default=True, help='Run Tile digits monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--channel-mon', dest='channel_mon', default=True, help='Run Tile raw channles monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--channel-time-mon', dest='channel_time_mon', default=None, help='Run Tile raw channles time monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--tmdb-mon', dest='tmdb_mon', default=True, help='Run TMDB monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--tmdb-digits-mon', dest='tmdb_digits_mon', default=True, help='Run TMDB digits monitoring', action=argparse.BooleanOptionalAction)
    mon.add_argument('--tmdb-channel-mon', dest='tmdb_channel_mon', default=True, help='Run TMDB raw channels monitoring', action=argparse.BooleanOptionalAction)

    # Set up Tile run type
    run_type_group = parser.add_argument_group('Tile Run Type')
    run_type = run_type_group.add_mutually_exclusive_group()
    run_type.add_argument('--cis', action='store_true', help='Tile CIS run type')
    run_type.add_argument('--laser', action='store_true', help='Tile laser run type')
    run_type.add_argument('--pedestals', action='store_true', help='Tile pedestals run type')
    run_type.add_argument('--physics', action='store_true', help='Tile physics run type')

    # Set up Tile reconstuction method
    method = parser.add_argument_group('Tile reconstuction method')
    method.add_argument('--opt2', default=True, help='Use Tile Opt2 reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--opt-atlas', dest='opt_atlas', default=True, help='Use Tile OptATLAS reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--fit', default=True, help='Use Tile Fit reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--of1', default=False, help='Use Tile OF1 reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--mf', default=False, help='Use Tile MF reconstuction method', action=argparse.BooleanOptionalAction)

    run_period_group = parser.add_argument_group('LHC Run period')
    run_period = run_period_group.add_mutually_exclusive_group()
    run_period.add_argument('--run2', action='store_true', help='LHC Run2 period')
    run_period.add_argument('--run3', action='store_true', help='LHC Run3 period')

    parser.add_argument('--upd4', action='store_true', help='Use UPD4 conditions')

    return parser


def useTagFor(tag, folder):
    ''' Function to override tag for DB folder (it should be used in --postExec) '''
    fullTag = f'{folder.title().replace("/","")}-{tag}' if folder.startswith('/TILE') and not tag.startswith('Tile') else tag
    from AthenaCommon.Logging import log
    log.info(f'Use tag {fullTag} for folder: {folder}')
    cfg.getService('IOVDbSvc').overrideTags += [f'<prefix>{folder}</prefix> <tag>{fullTag}</tag>']

def useSqliteFor(sqliteFile, folder):
    ''' Function to use Sqlite file for DB folder (it should be used in --postExec) '''
    iovDbSvc = cfg.getService('IOVDbSvc')
    from AthenaCommon.Logging import log
    log.info(f'Use Sqlite file {sqliteFile} for folder: {folder}')
    iovDbSvc.overrideTags += [f'<prefix>{folder}</prefix> <db>sqlite://;schema={sqliteFile};dbname={iovDbSvc.DBInstance}</db>']

def useSqliteAndTagFor(sqliteFile, tag, folder):
    ''' Function to use tag from Sqlite file for DB folder (it should be used in --postExec) '''
    useSqliteFor(sqliteFile, folder)
    useTagFor(tag, folder)

if __name__=='__main__':
    import sys,os

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    log.setLevel(Constants.INFO)

    parser = getArgumentParser(flags)
    args, _ = parser.parse_known_args()

    # Initially the following flags are not set up (they must be provided)
    flags.Input.Files = []
    flags.Tile.RunType = TileRunType.UNDEFINED

    # Initial configuration flags from command line arguments (to be used to set up defaults)
    flags.fillFromArgs(parser=parser)

    # =======>>> Set up the Athena configuration flags to defaults (can be overriden via comand line)

    # Set up the Tile input files
    if not flags.Input.Files and args.run:
        flags.Input.Files = TileInputFiles.findFilesFromAgruments(args)
    if not flags.Input.Files:
        log.error('Input files must be provided! For example: --filesInput=file1,file2,... or --run RUNNUMBER')
        sys.exit(-1)

    # Set up the Tile run type using arguments if it was not set up via configuration flags
    if flags.Tile.RunType is TileRunType.UNDEFINED:
        if args.cis:
            flags.Tile.RunType = TileRunType.CIS
        elif args.laser:
            flags.Tile.RunType = TileRunType.LAS
        elif args.pedestals:
            flags.Tile.RunType = TileRunType.PED
        elif args.physics:
            flags.Tile.RunType = TileRunType.PHY
        else:
            log.error('The Tile Run Type must be provided! For example: --laser or --cis, ..., or Tile.RunType=TileRunType.PED')
            sys.exit(-1)

    if flags.Tile.RunType not in [TileRunType.PHY, TileRunType.CIS]:
        flags.Exec.SkipEvents = 1
    elif flags.Tile.RunType is TileRunType.CIS:
        flags.Exec.SkipEvents = 192 # skip all events when just one channel is fired (4*48)

    # Set up Tile reconstuction method
    flags.Tile.doOpt2 = args.opt2
    flags.Tile.doOptATLAS = args.opt_atlas
    flags.Tile.doFit = args.fit
    flags.Tile.doOF1 = args.of1
    flags.Tile.doMF = args.mf

    flags.Tile.BestPhaseFromCOOL = True
    flags.Tile.NoiseFilter = 0 # disable noise filter by default
    flags.Tile.doOverflowFit = False
    flags.Tile.correctAmplitude = False
    flags.Tile.correctTime = flags.Tile.RunType in [TileRunType.PHY, TileRunType.GAPLAS, TileRunType.GAPCIS]
    flags.Tile.OfcFromCOOL = flags.Tile.RunType in [TileRunType.PHY, TileRunType.PED]

    runNumber = flags.Input.RunNumbers[0]

    # Set up LHC Run period
    if not any([args.run2, args.run3]):
        if not flags.Input.isMC:
            if runNumber >= 411938:
                args.run3 = True
            elif any([args.year and args.year > 2014, runNumber > 232000, flags.Input.ProjectName.startswith("data15_")]):
                args.run2 = True

    # Set up the DB global conditions tag
    if flags.Input.Format is Format.BS:
        if args.run3:
            condDbTag = 'CONDBR2-BLKPA-2023-01' if args.upd4 else 'CONDBR2-ES1PA-2023-01'
            detDescrVersion = 'ATLAS-R3S-2021-03-01-00'
        elif args.run2:
            condDbTag = 'CONDBR2-BLKPA-2018-16' if args.upd4 else 'CONDBR2-ES1PA-2018-05'
            detDescrVersion = 'ATLAS-R2-2016-01-00-01'
        else:
            condDbTag = 'COMCOND-BLKPA-RUN1-06' if (args.upd4 and runNumber > 141066) else 'COMCOND-ES1PA-006-05'
            detDescrVersion = 'ATLAS-R1-2012-03-02-00'

        flags.IOVDb.GlobalTag = condDbTag
        flags.GeoModel.AtlasVersion = detDescrVersion

    if args.mon:
        flags.DQ.useTrigger = False
        flags.DQ.enableLumiAccess = False
        if not flags.Output.HISTFileName:
            flags.Output.HISTFileName = f'{args.outputDirectory}/tilemon_{runNumber}_{args.version}.root'

    if args.tmdb is None:
        args.tmdb = not flags.Input.isMC

    if args.channel_time_mon is None:
        args.channel_time_mon = flags.Tile.RunType in [TileRunType.GAPLAS, TileRunType.GAPCIS] and (args.run2 or args.run3) and flags.Tile.doFit

    # Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    if args.dumpArguments:
        log.info('=====>>> FINAL ARGUMENTS FOLLOW:')
        print('{:40} : {}'.format('Argument Name', 'Value'))
        for a,v in (vars(args)).items():
            print(f'{a:40} : {v}')
        sys.exit(0)

    flags.needFlagsCategory('Tile')

    # Set up perfmon
    if args.perfmon:
        flags.PerfMon.doFullMonMT=True

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    flags.lock()

    log.info('=====>>> FINAL CONFIG FLAGS SETTINGS FOLLOW:')
    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    biGainRun = True if flags.Tile.RunType in [TileRunType.CIS, TileRunType.PED] else False

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Add perfmon
    if args.perfmon:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    # =======>>> Set up the File (BS | POOL) reading
    if flags.Input.Format is Format.BS:
        # Configure reading the Tile BS files
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=args.ntuple,
                                         readMuRcvDigits=any([args.tmdb_digits_mon, args.tmdb_mon, args.ntuple]),
                                         readMuRcvRawCh=any([args.tmdb_channel_mon, args.tmdb_mon, args.ntuple])) )

    else:
        # Configure reading POOL files
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    # =======>>> Set up the Tile raw channel maker only if readDigits is set
    if flags.Tile.readDigits:
        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        cfg.merge( TileRawChannelMakerCfg(flags) )
        rawChMaker = cfg.getEventAlgo('TileRChMaker')
        if args.threads and (args.threads > 1):
            rawChMaker.Cardinality = args.threads
        for builderTool in rawChMaker.TileRawChannelBuilder:
            builderTool.UseDSPCorrection = not biGainRun

    # =======>>> Set up the Tile Ntuple
    if args.ntuple:
        ntupleFile = f'{args.outputDirectory}/tile_{runNumber}_{args.version}.aan.root'
        from TileRec.TileAANtupleConfig import TileAANtupleCfg
        cfg.merge( TileAANtupleCfg(flags, outputFile=ntupleFile) )
        tileNtuple = cfg.getEventAlgo('TileNtuple')
        # CompressionSettings: algorithm * 100 + level
        tileNtuple.CompressionSettings = 204
        tileNtuple.SkipEvents = 4 if flags.Tile.RunType is TileRunType.LAS else 0
        tileNtuple.TileRawChannelContainerOpt = "TileRawChannelOpt2" if flags.Tile.doOpt2 else ""
        tileNtuple.TileRawChannelContainerDsp = "" if biGainRun else "TileRawChannelCnt"
        if args.reduced_ntuple:
            tileNtuple.Reduced = True
            tileNtuple.TileRawChannelContainer = ""
        if flags.Tile.RunType is TileRunType.LAS:
            tileNtuple.OfflineUnits = 1 # use pCb units for ntuple
        if flags.Tile.RunType in [TileRunType.GAPLAS, TileRunType.GAPCIS]:
            tileNtuple.TileDigitsContainerFlt = "TileDigitsCnt"
            tileNtuple.TileDigitsContainer = "" # do not save various error bits

    # =======>>> Set up the Tile monitoring
    if args.mon:

        def setOnlineEnvironment(alg):
            for tool in alg.GMTools:
                tool.Histograms = [h.replace('OFFLINE','ONLINE') for h in tool.Histograms]

        if args.digits_mon:
            from TileMonitoring.TileDigitsMonitorAlgorithm import TileDigitsMonitoringConfig
            cfg.merge(TileDigitsMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileDigitsMonAlg'))

        if args.channel_mon:
            from TileMonitoring.TileRawChannelMonitorAlgorithm import TileRawChannelMonitoringConfig
            cfg.merge(TileRawChannelMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileRawChannelMonAlg'))

        if args.channel_time_mon:
            from TileMonitoring.TileRawChannelTimeMonitorAlgorithm import TileRawChannelTimeMonitoringConfig
            cfg.merge(TileRawChannelTimeMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileRawChanTimeMonAlg'))

        if args.tmdb_digits_mon:
            from TileMonitoring.TileTMDBDigitsMonitorAlgorithm import TileTMDBDigitsMonitoringConfig
            cfg.merge(TileTMDBDigitsMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileTMDBDigitsMonAlg'))

        if args.tmdb_channel_mon:
            from TileMonitoring.TileTMDBRawChannelMonitorAlgorithm import TileTMDBRawChannelMonitoringConfig
            cfg.merge(TileTMDBRawChannelMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileTMDBRawChanDspMonAlg'))

        if args.tmdb_mon:
            from TileMonitoring.TileTMDBMonitorAlgorithm import TileTMDBMonitoringConfig
            cfg.merge(TileTMDBMonitoringConfig(flags))
            setOnlineEnvironment(cfg.getEventAlgo('TileTMDBMonAlg'))

        if any([args.tmdb_digits_mon, args.tmdb_mon]) and args.postprocessing:
            from AthenaCommon.Utils.unixtools import find_datafile
            configurations = []
            dataPath = find_datafile('TileMonitoring')
            if any([args.tmdb_digits_mon, args.tmdb_mon]):
                configurations += [os.path.join(dataPath, 'TileTMDBPostProc.yaml')]
            if args.digits_mon:
                configurations += [os.path.join(dataPath, 'TileDigitsPostProc.yaml')]
            if args.channel_mon:
                if flags.Tile.RunType in [TileRunType.CIS]:
                    configurations += [os.path.join(dataPath, 'TileRawChanCisPostProc.yaml')]
                else:
                    configurations += [os.path.join(dataPath, 'TileRawChanPostProc.yaml')]
                    if flags.Tile.RunType is TileRunType.LAS:
                        configurations += [os.path.join(dataPath, 'TileRawChanLasPostProc.yaml')]
                    if not biGainRun:
                        configurations += [os.path.join(dataPath, 'TileRawChanDspPostProc.yaml')]

            from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
            class TileMonPostProcessingAlg(DQPostProcessingAlg):
                def initialize(self):
                    if hasattr(self, 'OutputLevel'):
                        self.msg.setLevel(self.OutputLevel)
                    return super(TileMonPostProcessingAlg, self).initialize()

            ppa = TileMonPostProcessingAlg("TileMonPostProcessingAlg")
            ppa.OutputLevel = flags.Exec.OutputLevel
            ppa.ExtraInputs = [( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]
            ppa.Interval = 1000000 # Big number (>evtMax) to do postprocessing during finalization
            ppa.ConfigFiles = configurations
            ppa._ctr = 1 # Start postprocessing only after specified number of events (not during the first one)
            ppa.FileKey = f'/{flags.DQ.FileKey}/'

            cfg.addEventAlgo(ppa, sequenceName='AthEndSeq')


    # =======>>> Set up the Tile calibration
    if args.calib:
        if flags.Tile.RunType is TileRunType.LAS:
            laserCalibFile = f'tileCalibLAS_{runNumber}_{args.version}.root'
            from TileCalibAlgs.TileLaserCalibAlgConfig import TileLaserCalibAlgCfg
            cfg.merge( TileLaserCalibAlgCfg(flags, FileName=laserCalibFile) )

        elif flags.Tile.RunType is TileRunType.CIS:
            cisCalibFile = f'tileCalibCIS_{runNumber}_{args.version}.root'
            from TileCalibAlgs.TileCisCalibAlgConfig import TileCisCalibAlgCfg
            cfg.merge( TileCisCalibAlgCfg(flags, FileName=cisCalibFile) )

        elif flags.Tile.RunType in [TileRunType.PHY, TileRunType.PED]:
            defaultVersions = ['0', 'Ped.0', 'Ped']

            fileVersion = f'_{flags.Tile.NoiseFilter}' if flags.Tile.NoiseFilter > 0 else ""
            if args.version not in defaultVersions:
                fileVersion = f'_{args.version}_tnf{flags.Tile.NoiseFilter}'

            from TileCalibAlgs.TileRawChNoiseCalibAlgConfig import TileRawChNoiseCalibAlgCfg
            cfg.merge( TileRawChNoiseCalibAlgCfg(flags) )
            rawChanNoiseCalibAlg = cfg.getEventAlgo('TileRawChNoiseCalibAlg')
            rawChanNoiseCalibAlg.FileNamePrefix = f'{args.outputDirectory}/RawCh_NoiseCalib{fileVersion}'
            if flags.Input.isMC:
                rawChanNoiseCalibAlg.doFit = False
                rawChanNoiseCalibAlg.doFixed = False
                rawChanNoiseCalibAlg.doOpt = False
                rawChanNoiseCalibAlg.doDsp = True
                rawChanNoiseCalibAlg.UseforCells = 3 # i.e. from TileRawChannelCnt (like DSP)
            else:
                rawChanNoiseCalibAlg.doDsp = (flags.Tile.RunType is TileRunType.PHY)
                rawChanNoiseCalibAlg.UseforCells = 1 # 1= Fixed , 2= Opt2

            # Produce digi noise ntuple only for default version
            if args.version in defaultVersions:
                from TileCalibAlgs.TileDigiNoiseCalibAlgConfig import TileDigiNoiseCalibAlgCfg
                cfg.merge( TileDigiNoiseCalibAlgCfg(flags) )
                digiNoiseCalibAlg = cfg.getEventAlgo('TileDigiNoiseCalibAlg')
                digiNoiseCalibAlg.DoAvgCorr = False # False=> Full AutoCorr matrix calculation
                rawChanNoiseCalibAlg.FileNamePrefix = f'{args.outputDirectory}/Digi_NoiseCalib{fileVersion}'

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=args.printDetailedConfig)

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
