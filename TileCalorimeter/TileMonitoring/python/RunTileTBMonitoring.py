#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file RunTileTBMonitoring.py
@brief Script to run Tile TestBeam Reconstrcution/Monitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format, BeamType
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from TileConfiguration.TileConfigFlags import TileRunType

import os
import sys


def configureFlagsAndArgsFromPartition(flags, args, partition, log):
    """
    Configure the flags and args from partition in online

    Configure the following flags from partition in online:
        run type, run number, beam type, beam energy, project

    Configure the following args from partition in online: args.nsamples
    """

    from ipc import IPCPartition
    from ispy import ISObject
    ipcPartition = IPCPartition(partition)
    if not ipcPartition.isValid():
        log.error('Partition: ' + ipcPartition.name() + ' is not valid')
        sys.exit(1)

    # Set up default values
    runType = 'Physics'
    beamType = 'collisions'
    beamEnergy = 200   # In TileTB: [GeV]
    runNumber = 2400000
    project = 'data_H8'

    try:
        runParams = ISObject(ipcPartition, 'RunParams.SOR_RunParams', 'RunParams')
    except Exception:
        log.warning(f'No Run Parameters in IS => Set defaults: partition: {partition}, beam type: {beamType}'
                    + f', beam energy: {beamEnergy}, run number: {runNumber}, project tag: {project}')
    else:
        runParams.checkout()
        beamType = runParams.beam_type
        beamEnergy = runParams.beam_energy
        runNumber = runParams.run_number
        project = runParams.T0_project_tag
        runType = runParams.run_type
        log.info(f'RUN CONFIGURATION: run type: {runType}, beam type: {beamType}'
                 + f', beam energy: {beamEnergy}, run number: {runNumber}, project: {project}')

    try:
        cisParams = ISObject(ipcPartition, 'TileParams.cispar', 'TileCISparameters')
    except Exception:
        log.info('Could not find Tile Parameters in IS')
    else:
        try:
            cisParams.checkout()
        except Exception:
            log.info("Could not get Tile Parameters from IS")
        else:
            log.info(f'TILE CONFIGURATION: CISPAR size: {len(cisParams.data)}')
            cispar = 'TILE CONFIGURATION: CISPAR: '
            for d in cisParams.data:
                cispar += ' ' + str(d)
            log.info(cispar)

        if len(cisParams.data) == 16:
            data = cisParams.data
            if data[12] == 1:
                runType = 'Physics'
            elif data[12] == 2:
                runType = 'Laser'
            elif data[12] == 4:
                runType = 'Pedestals'
            elif data[12] == 8:
                runType = 'CIS'

            log.info(f'TILE CONFIGURATION: RunType: {runType}, Mode: {data[0]}, Samples: {data[1]}, Pipeline: {data[2]}'
                     + f', I3Delay: {data[3]}, Event: {data[4]}, Phase: {data[5]}, DAC: {data[6]}, Capacity: {data[7]}')

    # Try to get number of samples from partition
    nSamples = 15  # Default number of samples
    try:
        dspConfig = ISObject(ipcPartition, 'TileParams.TileCal_DSPConfig', 'TileCal_IS_DSPConfig')
    except Exception:
        log.info("Could not find Tile DSP Config in IS => set default number of samples to {nSamples}")
    else:
        try:
            dspConfig.checkout()
        except Exception:
            log.info("Could not get Tile DSP Config from IS => set default number of samples to {nSamples}")
        else:
            nSamples = dspConfig.samples
            log.info("Set number of samples from DSP Config in IS: {nSamples}")

    if 'Physics' in runType:
        flags.Tile.RunType = TileRunType.PHY
    elif 'CIS' in runType:
        flags.Tile.RunType = TileRunType.MONOCIS if 'mono' in runType else TileRunType.CIS
    elif 'Laser' in runType:
        flags.Tile.RunType = TileRunType.LAS
    elif 'Pedestals' in runType:
        flags.Tile.RunType = TileRunType.PED

    flags.Beam.Type = BeamType(beamType)
    flags.Beam.Energy = beamEnergy
    flags.Input.ProjectName = project
    flags.Input.RunNumbers = [runNumber]
    args.nsamples = nSamples


def TileTestBeamMonitoringCfg(flags, fragIDs=[0x100, 0x101, 0x200, 0x201, 0x402], **kwargs):

    ''' Function to configure Tile TestBeam monitoring.'''

    acc = ComponentAccumulator()

    from TileMonitoring.TileTBBeamMonitorAlgorithm import TileTBBeamMonitoringConfig
    acc.merge(TileTBBeamMonitoringConfig(flags, fragIDs=fragIDs))

    from TileMonitoring.TileTBMonitorAlgorithm import TileTBMonitoringConfig
    acc.merge(TileTBMonitoringConfig(flags, fragIDs=fragIDs))

    from TileMonitoring.TileTBPulseMonitorAlgorithm import TileTBPulseMonitoringConfig
    acc.merge(TileTBPulseMonitoringConfig(flags, timeRange=[-200, 200], fragIDs=fragIDs))

    from TileMonitoring.TileTBCellMonitorAlgorithm import TileTBCellMonitoringConfig
    acc.merge(TileTBCellMonitoringConfig(flags, timeRange=[-200, 200], fragIDs=fragIDs))

    from TileMonitoring.TileDigitsFlxMonitorAlgorithm import TileDigitsFlxMonitoringConfig
    acc.merge(TileDigitsFlxMonitoringConfig(flags, TileDigitsContainerFlx="TileDigitsFlxFiltered"))

    from TileMonitoring.TileRawChannelFlxMonitorAlgorithm import TileRawChannelFlxMonitoringConfig
    acc.merge(TileRawChannelFlxMonitoringConfig(flags, TileRawChannelContainerFlx="TileRawChannelFlxFit"))

    return acc


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
    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--printConfig', action='store_true', help='Print detailed Athena configuration')
    parser.add_argument('--dumpArguments', action='store_true', help='Print arguments and exit')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201', '0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=15, help='Number of samples')
    parser.add_argument('--use-sqlite', dest='useSqlite', default='/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db',
                        help='Providing local SQlite file, conditions constants will be used from it')

    parser.add_argument('--stateless', action="store_true", help='Run Online Tile TB monitoring in partition')
    parser.add_argument('--partition', default="", help='EMON, Partition name, default taken from $TDAQ_PARTITION if not set')
    parser.add_argument('--key', type=str, default='ReadoutApplication', help='EMON, Selection key, e.g.: ReadoutApplication (TileTB)')
    parser.add_argument('--keyValue', default=['TileREB-ROS'], help='EMON, Key values, e.g.: TileREB-ROS (TileTB)')
    parser.add_argument('--keyCount', type=int, default=0, help='EMON, key count, e.g. 5 to get five random SFIs')
    parser.add_argument('--publishName', default='TilePT-stateless-tb', help='EMON, Name under which to publish histograms')
    parser.add_argument('--include', default="", help='EMON, Regular expression to select histograms to publish')
    parser.add_argument('--lvl1Items', default=[], help='EMON, A list of L1 bit numbers, default []')
    parser.add_argument('--lvl1Names', default=[], help='EMON, A list of L1 bit names, default []')
    parser.add_argument('--lvl1Logic', default='Ignore', choices=['And','Or','Ignore'], help='EMON, default: Ignore')
    parser.add_argument('--lvl1Origin', default='TAV', choices=['TBP','TAP','TAV'], help='EMON, default: TAV')
    parser.add_argument('--streamType', default='physics', help='EMON, HLT stream type (e.g. physics or calibration)')
    parser.add_argument('--streamNames', default=['tile'], help='EMON, List of HLT stream names')
    parser.add_argument('--streamLogic', default='Ignore', choices=['And','Or','Ignore'], help='EMON, default: Ignore')
    parser.add_argument('--triggerType', type=int, default=256, help='EMON, LVL1 8 bit trigger type, default: 256')
    parser.add_argument('--groupName', default="TileTBMon", help='EMON, Name of the monitoring group')

    update_group = parser.add_mutually_exclusive_group()
    update_group.add_argument('--frequency', type=int, default=0, help='EMON, Frequency (in number of events) of publishing histograms')
    update_group.add_argument('--updatePeriod', type=int, default=30, help='EMON, Frequency (in seconds) of publishing histograms')

    args, _ = parser.parse_known_args()

    if args.dumpArguments:
        log.info('=====>>> FINAL ARGUMENTS FOLLOW')
        print('{:40} : {}'.format('Argument Name', 'Value'))
        for a,v in (vars(args)).items():
            print(f'{a:40} : {v}')
        sys.exit(0)

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    # Initially the following flags are not set up (they must be provided)
    flags.Input.Files = []

    # Initial configuration flags from command line arguments (to be used to set up defaults)
    flags.fillFromArgs(parser=parser)

    # =======>>> Set the Athena configuration flags to defaults (can be overriden via comand line)
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.Common.isOnline = True
    flags.GeoModel.AtlasVersion = 'ATLAS-R2-2015-04-00-00'

    flags.Tile.doFit = True
    flags.Tile.useDCS = False
    flags.Tile.NoiseFilter = 0
    flags.Tile.correctTime = False
    flags.Tile.correctTimeJumps = False
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.doOverflowFit = False

    flags.Exec.PrintAlgsSequence = True

    if args.stateless:
        flags.Input.isMC = False
        flags.Input.Format = Format.BS
        partition = args.partition if args.partition else os.getenv('TDAQ_PARTITION', 'TileTB')
        configureFlagsAndArgsFromPartition(flags, args, partition, log)
    else:
        flags.Tile.RunType = TileRunType.PHY
        flags.Beam.Type = BeamType.Collisions
        # Get beam energy from meta data (Tile TB setup: [GeV])
        flags.Beam.Energy = GetFileMD(flags.Input.Files).get("beam_energy", 100)

        if not (args.filesInput or flags.Input.Files):
            flags.Input.Files = defaultTestFiles.RAW_RUN2

    # =======>>> Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    if not flags.Output.HISTFileName:
        runNumber = flags.Input.RunNumbers[0]
        flags.Output.HISTFileName = f'tiletbmon_{runNumber}.root'

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    flags.lock()

    log.info('=====>>> FINAL CONFIG FLAGS SETTINGS FOLLOW:')
    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    # =======>>> Initialize configuration object, add accumulator, merge, and run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # =======>>> Configure Tile raw data (digits) reading
    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                     readDigits=True,
                                     readRawChannel=True,
                                     readDigitsFlx=True,
                                     readBeamElem=True,
                                     stateless=args.stateless) )

    if args.stateless:
        bsEmonInputSvc = cfg.getService('ByteStreamInputSvc')
        bsEmonInputSvc.Partition = args.partition
        bsEmonInputSvc.Key = args.key
        bsEmonInputSvc.KeyValue = args.keyValue
        bsEmonInputSvc.KeyCount = args.keyCount
        bsEmonInputSvc.PublishName = args.publishName
        bsEmonInputSvc.ISServer = 'Histogramming'
        bsEmonInputSvc.UpdatePeriod = args.updatePeriod
        bsEmonInputSvc.Frequency = args.frequency
        bsEmonInputSvc.LVL1Items = args.lvl1Items
        bsEmonInputSvc.LVL1Names = args.lvl1Names
        bsEmonInputSvc.LVL1Logic = args.lvl1Logic
        bsEmonInputSvc.LVL1Origin = args.lvl1Origin
        bsEmonInputSvc.StreamType = args.streamType
        bsEmonInputSvc.StreamNames = args.streamNames
        bsEmonInputSvc.StreamLogic = args.streamLogic
        bsEmonInputSvc.GroupName = args.groupName
        bsEmonInputSvc.ProcessCorruptedEvents = True
        bsEmonInputSvc.BufferSize = 2000

    # =======>>> Configure reconstruction of Tile TestBeam data
    from TileTBRec.TileTestBeamRecoConfig import TileTestBeamRecoCfg
    cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=args.nsamples) )
    cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=16, useFELIX=True) )

    if args.useSqlite:
        cfg.getService('IOVDbSvc').overrideTags += [
            f'<prefix>/TILE</prefix> <db>sqlite://;schema={args.useSqlite};dbname={flags.IOVDb.DatabaseInstance}</db>',
            # ROD folder does not exist in Sqlite file at the moment (should be added)
            f'<prefix>/TILE/ONL01/STATUS/ROD</prefix> <db>COOLONL_TILE/{flags.IOVDb.DatabaseInstance}</db>'
        ]

    # =======>>> Configure Tile TestBeam monitoring
    cfg.merge(TileTestBeamMonitoringCfg(flags, fragIDs=fragIDs))

    # =======>>> Configure ROD to ROB mapping
    # Scan first event for all fragments to create proper ROD to ROB map
    cfg.getCondAlgo('TileHid2RESrcIDCondAlg').RODStatusProxy = None

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    if args.printConfig:
        cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

    if args.config_only:
        cfg.store(open('TileTestBeamMonitoring.pkl', 'wb'))
    else:
        sc = cfg.run()
        # Success should be 0
        sys.exit(not sc.isSuccess())
