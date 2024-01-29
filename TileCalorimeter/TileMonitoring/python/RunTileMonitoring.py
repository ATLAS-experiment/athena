#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''@file RunTileMonitoring.py
@brief Script to run Tile Reconstrcution/Monitoring with new-style configuration
'''

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, Format
from TileConfiguration.TileConfigFlags import TileRunType

def _configFlagsFromPartition(flags, partition, log):
    """
    Configure the following flags from partition in online: run number, beam type, and project
    """

    from ipc import IPCPartition
    from ispy import ISObject
    ipcPartition = IPCPartition(partition)
    if not ipcPartition.isValid():
        log.error( 'Partition: ' + ipcPartition.name() + ' is not valid' )
        sys.exit(1)
    try:
        runParams = ISObject(ipcPartition, 'RunParams.SOR_RunParams', 'RunParams')
    except Exception:
        beamType = 'cosmics'
        runNumber = 399999
        projectName = 'data20_calib'
        log.warning("No Run Parameters in IS => Set defaults: partition: %s, beam type: %i, run number: %i, project tag: %s",
                    partition, beamType, runNumber, projectName)
    else:
        runParams.checkout()
        beamType = runParams.beam_type
        beamEnergy = runParams.beam_energy
        runNumber = runParams.run_number
        projectName = runParams.T0_project_tag
        runType = runParams.run_type
        log.info("RUN CONFIG: partition: %s, run type: %s, beam type: %i, beam energy: %i, run number: %i, project tag: %s",
                 partition, runType, beamType, beamEnergy, runNumber, projectName)

        if any([projectName.endswith(_) for _ in ("cos", "test", "calib")]):
            beamType = 'cosmics'
        elif projectName.endswith('1beam'):
            beamType = 'singlebeam'
        elif beamEnergy > 0:
            beamType = 'collisions'
        else:
           beamType = 'cosmics'

        if partition == 'Tile':
            flags.Tile.NoiseFilter = 0
            if 'CIS' in runType:
                flags.Tile.RunType = TileRunType.MONOCIS if 'mono' in runType else TileRunType.CIS
            elif 'Laser' in runType:
                flags.Tile.RunType = TileRunType.LAS
            elif 'Pedestals' in runType:
                flags.Tile.RunType = TileRunType.PED

    flags.Beam.Type = BeamType(beamType)
    flags.Input.ProjectName = projectName
    flags.Input.RunNumbers = [runNumber]


if __name__=='__main__':
    import sys,os

    def _addBoolArgument(parser, argument, dest=None, help=''):
        group = parser.add_mutually_exclusive_group()
        destination = dest if dest else argument
        group.add_argument('--' + argument, dest=destination, action='store_true', help="Switch on " + help)
        group.add_argument('--no-' + argument, dest=destination, action='store_false', help="Switch off " + help)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--printDetailedConfig', action='store_true', help='Print detailed Athena configuration')
    parser.add_argument('--dumpArguments', action='store_true', help='Print arguments and exit')

    _addBoolArgument(parser, 'laser', help='Tile Laser monitoring')
    _addBoolArgument(parser, 'cis', help='Tile CIS monitoring')
    _addBoolArgument(parser, 'noise', help='Tile Noise monitoring')
    _addBoolArgument(parser, 'cells', help='Tile Calorimeter Cells monitoring')
    _addBoolArgument(parser, 'towers', help='Tile Calorimeter Towers monitoring')
    _addBoolArgument(parser, 'clusters', help='Tile Calorimeter Clusters monitoring')
    _addBoolArgument(parser, 'muid', help='Tile Calorimeter MuId monitoring')
    _addBoolArgument(parser, 'muonfit', help='Tile Calorimeter MuonFit monitoring')
    _addBoolArgument(parser, 'mbts', help='MBTS monitoring')
    _addBoolArgument(parser, 'rod', help='Tile Calorimeter ROD monitoring')
    _addBoolArgument(parser, 'digi-noise',dest='digiNoise', help='Tile digi noise monitoring')
    _addBoolArgument(parser, 'raw-chan-noise',dest='rawChanNoise', help='Tile raw channel noise monitoring')
    _addBoolArgument(parser, 'tmdb', help='TMDB monitoring')
    _addBoolArgument(parser, 'tmdb-digits', dest='tmdbDigits', help='TMDB digits monitoring')
    _addBoolArgument(parser, 'tmdb-raw-channels', dest='tmdbRawChannels', help='TMDB raw channels monitoring')
    _addBoolArgument(parser, 'online', help='Online environment running')

    parser.add_argument('--stateless', action="store_true", help='Run Online Tile monitoring in partition')
    parser.add_argument('--use-mbts-trigger', action="store_true", dest='useMbtsTrigger', help='Use L1 MBTS triggers')
    parser.add_argument('--partition', default="", help='EMON, Partition name, default taken from $TDAQ_PARTITION if not set')
    parser.add_argument('--key', type=str, default="",
                        help='EMON, Selection key, e.g.: SFI, default: dcm (ATLAS), CompleteEvent (TileMon), ReadoutApplication (Tile)')
    parser.add_argument('--keyValue', default="",
                        help='EMON, Key values, e.g. [SFI-1, SFI-2]; if empty all SFIs; default: "" (*), TileREB-ROS (Tile)')
    parser.add_argument('--keyCount', type=int, default=50,
                        help='EMON, key count, e.g. 5 to get five random SFIs, default: 50 (physics), 1000 (laser:CIS)')
    parser.add_argument('--publishName', default='TilePT-stateless-10', help='EMON, Name under which to publish histograms')
    parser.add_argument('--include', default="", help='EMON, Regular expression to select histograms to publish')
    parser.add_argument('--lvl1Items', default=[], help='EMON, A list of L1 bit numbers, default []')
    parser.add_argument('--lvl1Names', default=[], help='EMON, A list of L1 bit names, default []')
    parser.add_argument('--lvl1Logic', default='Ignore', choices=['And','Or','Ignore'], help='EMON, default: Ignore')
    parser.add_argument('--lvl1Origin', default='TAV', choices=['TBP','TAP','TAV'], help='EMON, default: TAV')
    parser.add_argument('--streamType', default='physics', help='EMON, HLT stream type (e.g. physics or calibration)')
    parser.add_argument('--streamNames', default=['express','Main','Standby','CosmicCalo','L1Calo','ZeroBias','Background','MinBias','CosmicMuons','IDCosmic'], help='EMON, List of HLT stream names')
    parser.add_argument('--streamLogic', default='Or', choices=['And','Or','Ignore'], help='EMON, default: Or')
    parser.add_argument('--triggerType', type=int, default=256, help='EMON, LVL1 8 bit trigger type, default: 256')
    parser.add_argument('--groupName', default="TilePhysMon", help='EMON, Name of the monitoring group')
    parser.add_argument('--postProcessingInterval', type=int, default=10000000,
                        help='Number of events between postprocessing steps (<0: disabled, >evtMax: during finalization)')
    parser.add_argument('--perfmon', action='store_true', help='Run perfmon')

    update_group = parser.add_mutually_exclusive_group()
    update_group.add_argument('--frequency', type=int, default=0, help='EMON, Frequency (in number of events) of publishing histograms')
    update_group.add_argument('--updatePeriod', type=int, default=60, help='EMON, Frequency (in seconds) of publishing histograms')

    args, _ = parser.parse_known_args()

    # Set up default arguments which can be overriden via command line
    if not any([args.laser, args.cis, args.noise, args.mbts]):
        mbts = False if (args.stateless and args.useMbtsTrigger) else True
        parser.set_defaults(cells=True, towers=True, clusters=True, muid=True, muonfit=True, mbts=mbts,
                            rod=True, tmdb=True, tmdbDigits=True, tmdbRawChannels=True)
    elif args.noise:
        parser.set_defaults(digiNoise=True, rawChanNoise=True)

    if args.stateless:
        parser.set_defaults(online=True)
        partition = args.partition if args.partition else os.getenv('TDAQ_PARTITION', 'ATLAS')

        keys = {'ATLAS' : 'dcm', 'TileMon' : 'CompleteEvent', 'Tile' : 'ReadoutApplication'}
        key = args.key if args.key else keys.get(partition, 'dcm')

        keyValues = {'Tile': 'TileREB-ROS'}
        keyValue =  args.keyValue if  args.keyValue else keyValues.get(partition, "")

        # Given frequency, set up updatePeriod to 0, since updatePeriod has higher priority
        updatePeriod = 0 if args.frequency > 0 else args.updatePeriod
        parser.set_defaults(partition=partition, key=key, keyValue=keyValue, updatePeriod=updatePeriod)

        if any([args.laser, args.cis]):
            calibGroupName = 'TileLasMon' if args.laser else 'TileCisMon'
            parser.set_defaults(streamType='calibration', streamNames=['Tile'], streamLogic='And', keyCount=1000, groupName=calibGroupName)
        elif args.noise:
            publishInclude = ".*Summary.*|.*DMUErrors.*|.*DigiNoise.*"
            parser.set_defaults(streamType='physics', streamNames=['CosmicCalo'], streamLogic='And', include=publishInclude,
                                triggerType=0x82, frequency=300, updatePeriod=0, keyCount=1000, groupName='TileNoiseMon', postProcessingInterval=299)
        elif args.mbts:

            _l1Items = []
            _l1Names = ['L1_MBTS_1', 'L1_MBTS_1_EMPTY', 'L1_MBTS_1_1_EMPTY']
            _l1Names += ['L1_MBTSA' + str(counter) for counter in range(0, 16)]
            _l1Names += ['L1_MBTSC' + str(counter) for counter in range(0, 16)]
            parser.set_defaults(lvl1Logic='Or', lvl1Origin='TBP', lvl1Items=_l1Items, lvl1Names=_l1Names,
                                keyCount=1000, groupName='TileMBTSMon', useMbtsTrigger = True)
        else:
            parser.set_defaults(postProcessingInterval=100)

    args, _ = parser.parse_known_args()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    if args.dumpArguments:
        log.info('=====>>> FINAL ARGUMENTS FOLLOW')
        print('{:40} : {}'.format('Argument Name', 'Value'))
        for a,v in (vars(args)).items():
            print(f'{a:40} : {v}')
        sys.exit(0)

    # Set the Athena configuration flags to defaults (can be overriden via comand line)
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Tile.RunType = TileRunType.PHY

    if args.mbts and args.useMbtsTrigger:
        flags.Trigger.triggerConfig = 'DB'

    if args.stateless:
        _configFlagsFromPartition(flags, args.partition, log)
        flags.Input.Files = []
        flags.Input.isMC = False
        flags.Input.Format = Format.BS
        if args.mbts and args.useMbtsTrigger:
            if args.partition in ['TileMon']:
                flags.Trigger.triggerConfig = 'DB:{:s}:{:d},{:d},{:d},{:d}'.format('TRIGGERDB_RUN3', 3185, 4357, 4219, 2543)
            else:
                beamType = flags.Beam.Type
                from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
                autoConfigOnlineRecoFlags(flags, args.partition)
                flags.Beam.Type = beamType

    else:
        if args.filesInput:
            flags.Input.Files = args.filesInput.split(",")
        elif args.laser:
            inputDirectory = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileByteStream/TileByteStream-02-00-00"
            inputFile = "data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data"
            flags.Input.Files = [os.path.join(inputDirectory, inputFile)]
            flags.Input.RunNumbers = [363899]
        elif args.cis:
            inputDirectory = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileByteStream/TileByteStream-02-00-00"
            inputFile = "data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data"
            flags.Input.Files = [os.path.join(inputDirectory, inputFile)]
            flags.Input.RunNumbers = [363899]
        elif args.noise:
            inputDirectory = 'root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/det-tile/test'
            inputFile = 'data12_8TeV.00201555.physics_ZeroBiasOverlay.merge.RAW._lb0150._SFO-ALL._0001.1'
            flags.Input.Files = [os.path.join(inputDirectory, inputFile)]
        else:
            from AthenaConfiguration.TestDefaults import defaultTestFiles
            flags.Input.Files = defaultTestFiles.RAW_RUN2

    runNumber = flags.Input.RunNumbers[0]
    flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00' if not flags.Input.isMC and runNumber >= 411938 else 'ATLAS-R2-2016-01-00-01'

    if not flags.Output.HISTFileName:
        flags.Output.HISTFileName = 'tilemon_{}.root'.format(runNumber)

    if args.online:
        flags.Common.isOnline = True
    if flags.Common.isOnline:
        flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01' if runNumber > 232498 else 'COMCOND-HLTP-004-02'
        flags.DQ.Environment = 'online'
        flags.DQ.FileKey = ''
    else:
        flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2023-01' if runNumber > 232498 else 'COMCOND-BLKPA-RUN1-06'

    if any([args.laser, args.cis]):
        if args.laser:
            flags.Tile.RunType = TileRunType.GAPLAS
        elif args.cis:
            flags.Tile.RunType = TileRunType.GAPCIS
        flags.Tile.doFit = True
        flags.Tile.correctTime = True
        flags.Tile.doOverflowFit = False
        flags.Tile.BestPhaseFromCOOL = True
        flags.Tile.NoiseFilter = 1

    # Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    # perfmon
    if args.perfmon:
        flags.PerfMon.doFullMonMT=True

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    log.info('=====>>> FINAL CONFIG FLAGS SETTINGS FOLLOW')
    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Add perfmon
    if args.perfmon:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    typeNames = ['CTP_RDO/CTP_RDO'] if args.mbts and args.useMbtsTrigger else []

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                     readMuRcvDigits=any([args.tmdbDigits, args.tmdb]),
                                     readMuRcvRawCh=any([args.tmdbRawChannels, args.tmdb]),
                                     readLaserObj=False,
                                     stateless=args.stateless,
                                     type_names=typeNames) )

    if args.stateless:
        bsEmonInputSvc = cfg.getService( "ByteStreamInputSvc" )
        bsEmonInputSvc.Partition = args.partition
        bsEmonInputSvc.Key = args.key
        bsEmonInputSvc.KeyValue = args.keyValue
        bsEmonInputSvc.KeyCount = args.keyCount
        bsEmonInputSvc.PublishName = args.publishName
        bsEmonInputSvc.ISServer = 'Histogramming'
        bsEmonInputSvc.Include = args.include
        bsEmonInputSvc.UpdatePeriod = args.updatePeriod
        bsEmonInputSvc.Frequency = args.frequency
        bsEmonInputSvc.LVL1Items = args.lvl1Items
        bsEmonInputSvc.LVL1Names = args.lvl1Names
        bsEmonInputSvc.LVL1Logic = args.lvl1Logic
        bsEmonInputSvc.LVL1Origin = args.lvl1Origin
        bsEmonInputSvc.StreamType = 'express' if flags.Beam.Type is BeamType.SingleBeam else args.streamType
        bsEmonInputSvc.StreamNames = args.streamNames
        bsEmonInputSvc.StreamLogic = args.streamLogic
        bsEmonInputSvc.GroupName = args.groupName
        bsEmonInputSvc.ProcessCorruptedEvents = True

    cfg.addPublicTool( CompFactory.TileROD_Decoder(fullTileMode = runNumber) )

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )
    if args.threads and (args.threads > 1):
        rawChMaker = cfg.getEventAlgo('TileRChMaker')
        rawChMaker.Cardinality = args.threads

    l1Triggers = ['bit0_RNDM', 'bit1_ZeroBias', 'bit2_L1Cal', 'bit3_Muon',
                  'bit4_RPC', 'bit5_FTK', 'bit6_CTP', 'bit7_Calib', 'AnyPhysTrig']

    if any([args.laser, args.cis]):
        triggerTypes = [0x34] if args.laser else [0x32]
        from TileMonitoring.TileRawChannelTimeMonitorAlgorithm import TileRawChannelTimeMonitoringConfig
        cfg.merge(TileRawChannelTimeMonitoringConfig(flags, TriggerTypes=triggerTypes))

    if args.rod:
        from TileMonitoring.TileRODMonitorAlgorithm import TileRODMonitoringConfig
        cfg.merge(TileRODMonitoringConfig(flags, fillHistogramsForL1Triggers = l1Triggers))

    if args.tmdbDigits:
        from TileMonitoring.TileTMDBDigitsMonitorAlgorithm import TileTMDBDigitsMonitoringConfig
        cfg.merge(TileTMDBDigitsMonitoringConfig(flags))

    if args.tmdbRawChannels:
        from TileMonitoring.TileTMDBRawChannelMonitorAlgorithm import TileTMDBRawChannelMonitoringConfig
        cfg.merge(TileTMDBRawChannelMonitoringConfig(flags))

    if args.tmdb:
        from TileMonitoring.TileTMDBMonitorAlgorithm import TileTMDBMonitoringConfig
        cfg.merge(TileTMDBMonitoringConfig(flags))

    if any([args.cells, args.towers, args.clusters, args.mbts, args.muid, args.muonfit]):
        from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
        cfg.merge( TileCellMakerCfg(flags) )

    if args.cells:
        from TileMonitoring.TileCellMonitorAlgorithm import TileCellMonitoringConfig
        cfg.merge(TileCellMonitoringConfig(flags, fillHistogramsForL1Triggers = l1Triggers, fillGapScintilatorHistograms=True))

    if args.towers:
        from TileMonitoring.TileTowerMonitorAlgorithm import TileTowerMonitoringConfig
        cfg.merge(TileTowerMonitoringConfig(flags, fillHistogramsForL1Triggers = l1Triggers))

    if args.clusters:
        from TileMonitoring.TileClusterMonitorAlgorithm import TileClusterMonitoringConfig
        cfg.merge(TileClusterMonitoringConfig(flags, fillTimingHistograms = True, fillHistogramsForL1Triggers = l1Triggers))

    if args.mbts:
        from TileMonitoring.TileMBTSMonitorAlgorithm import TileMBTSMonitoringConfig
        cfg.merge(TileMBTSMonitoringConfig(flags, FillHistogramsPerMBTS = True, useTrigger = args.useMbtsTrigger))

    if args.muid:
        from TileMuId.TileMuIdConfig import TileLookForMuAlgCfg
        cfg.merge(TileLookForMuAlgCfg(flags))

        from TileMonitoring.TileMuIdMonitorAlgorithm import TileMuIdMonitoringConfig
        cfg.merge(TileMuIdMonitoringConfig(flags, fillHistogramsForL1Triggers = l1Triggers))

    if args.muonfit:
        from TileCosmicAlgs.TileMuonFitterConfig import TileMuonFitterCfg
        cfg.merge(TileMuonFitterCfg(flags))

        from TileMonitoring.TileMuonFitMonitorAlgorithm import TileMuonFitMonitoringConfig
        cfg.merge(TileMuonFitMonitoringConfig(flags, fillHistogramsForL1Triggers = l1Triggers))

    if args.digiNoise:
        from TileMonitoring.TileDigiNoiseMonitorAlgorithm import TileDigiNoiseMonitoringConfig
        cfg.merge(TileDigiNoiseMonitoringConfig(flags))

    if args.rawChanNoise:
        from TileMonitoring.TileRawChannelNoiseMonitorAlgorithm import TileRawChannelNoiseMonitoringConfig
        cfg.merge(TileRawChannelNoiseMonitoringConfig(flags))

    from TileMonitoring.TileDQFragMonitorAlgorithm import TileDQFragMonitoringConfig
    cfg.merge( TileDQFragMonitoringConfig(flags) )

    if any([args.digiNoise, args.rawChanNoise, args.tmdbDigits, args.tmdb]) and args.postProcessingInterval > 0:
        from AthenaCommon.Utils.unixtools import find_datafile
        configurations = []
        dataPath = find_datafile('TileMonitoring')
        if any([args.tmdbDigits, args.tmdb]):
            configurations += [os.path.join(dataPath, 'TileTMDBPostProc.yaml')]
        if args.digiNoise:
            configurations += [os.path.join(dataPath, 'TileDigiNoisePostProc.yaml')]
        if args.rawChanNoise:
            configurations += [os.path.join(dataPath, 'TileRawChanNoisePostProc.yaml')]

        from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
        class TileMonPostProcessingAlg(DQPostProcessingAlg):
            def initialize(self):
                if hasattr(self, 'OutputLevel'):
                    self.msg.setLevel(self.OutputLevel)
                return super(TileMonPostProcessingAlg, self).initialize()

        ppa = TileMonPostProcessingAlg("TileMonPostProcessingAlg")
        ppa.OutputLevel = flags.Exec.OutputLevel
        ppa.ExtraInputs = {( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )}
        ppa.Interval = args.postProcessingInterval
        ppa.ConfigFiles = configurations
        ppa._ctr = 1 # Start postprocessing only after specified number of events (not during the first one)
        if flags.Common.isOnline:
            fileKey = flags.DQ.FileKey
            ppa.FileKey = (fileKey + '/') if not fileKey.endswith('/') else fileKey
        else:
            ppa.FileKey = f'/{flags.DQ.FileKey}/run_{runNumber}/'

        cfg.addEventAlgo(ppa, sequenceName='AthEndSeq')

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    if flags.Common.isOnline:
        cfg.getService("THistSvc").Output=["Tile DATAFILE='%s' OPT='RECREATE'" % (flags.Output.HISTFileName)]
        cfg.getService("TileCablingSvc").CablingType=6

    if args.stateless and args.cis:
        cfg.getEventAlgo('TileDQstatusAlg').TileBeamElemContainer=""

    cfg.printConfig(withDetails=args.printDetailedConfig)

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
