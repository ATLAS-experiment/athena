#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file RunTileTBRec.py
@brief Script to run Tile TestBeam Reconstruction/Monitoring
'''

from AthenaConfiguration.Enums import BeamType
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from TileConfiguration.TileConfigFlags import TileRunType
from TileRecEx import TileInputFiles
from AthenaCommon.SystemOfUnits import GeV

import sys

epiLog = """
Examples:

    RunTileTBRec.py --run RUNNUMBER --evtMax 1
    RunTileTBRec.py --filesInput=FILE1,FILE2 Exec.SkipEvents=100

At least one should provide the following arguments or Athena configuration flags (flags have higher priority):
   Input file(s), e.g.: --run RUNNUMBER | --filesInput=FILE1,FILE2 | Input.Files="['FILE1','FILE2']"
"""

if __name__ == '__main__':

    # Setup logs
    from AthenaCommon.Logging import logging
    log = logging.getLogger('RunTileTBRec')
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    import argparse

    flags = initConfigFlags()
    parserParents = [flags.getArgumentParser(), TileInputFiles.getArgumentParser(add_help=False)]
    parser = argparse.ArgumentParser(parents=parserParents, add_help=False, fromfile_prefix_chars='@', epilog=epiLog, formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--postInclude', nargs='+', help='Configuration fragment to include after main job options')
    parser.add_argument('--printConfig', action='store_true', help='Print detailed Athena configuration')
    parser.add_argument('--dumpArguments', action='store_true', help='Print arguments and exit')
    parser.add_argument('--outputVersion', type=str, default="", help='Version to be used in output files for ntuple and monitoring')
    parser.add_argument('--outputDirectory', default='.', help='Output directory for produced files')

    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x100', '0x101', '0x200', '0x201', '0x402'],
                        help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=None, help='Number of samples')
    parser.add_argument('--use-sqlite', dest='useSqlite', default='/afs/cern.ch/user/t/tiledemo/public/efmon/condb/tileSqlite.db',
                        help='Providing local SQlite file, conditions constants will be used from it')
    parser.add_argument('--mon', default=False, help='Run Tile TB monitoring', action=argparse.BooleanOptionalAction)
    parser.add_argument('--offline-units', type=int, choices=[0, 1, 2, 3], default=None,
                        help='Offline units in ntuple: 0 (ADC), 1 (pC), 2 (Cesium pC), 3 (MeV)')

    # Set up Tile run type
    run_type_group = parser.add_argument_group('Tile Run Type')
    run_type = run_type_group.add_mutually_exclusive_group()
    run_type.add_argument('--cis', action='store_true', help='Tile CIS run type')
    run_type.add_argument('--mono-cis', action='store_true', dest='mono_cis', help='Tile mono CIS run type')
    run_type.add_argument('--laser', action='store_true', help='Tile laser run type')
    run_type.add_argument('--pedestals', action='store_true', help='Tile pedestals run type')
    run_type.add_argument('--physics', action='store_true', help='Tile physics run type')

    # Set up Tile reconstuction method
    method = parser.add_argument_group('Tile reconstuction method')
    method.add_argument('--opt2', default=False, help='Use Tile Opt2 reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--opt-atlas', dest='opt_atlas', default=False, help='Use Tile OptATLAS reconstuction method', action=argparse.BooleanOptionalAction)
    method.add_argument('--fit', default=True, help='Use Tile Fit reconstuction method', action=argparse.BooleanOptionalAction)

    args, _ = parser.parse_known_args()

    if args.dumpArguments:
        log.info('=====>>> FINAL ARGUMENTS FOLLOW')
        print('{:40} : {}'.format('Argument Name', 'Value'))
        for a, v in (vars(args)).items():
            print(f'{a:40} : {v}')
        sys.exit(0)

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    # Initially the following flags are not set up (they must be provided)
    flags.Input.Files = []

    # Initial configuration flags from command line arguments (to be used to set up defaults)
    flags.fillFromArgs(parser=parser)

    # =======>>> Set the Athena configuration flags to defaults (can be overriden via comand line)
    flags.Exec.MaxEvents = 3
    flags.Common.isOnline = True
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.PrintAlgsSequence = True

    flags.Tile.doFit = True
    flags.Tile.useDCS = False
    flags.Tile.NoiseFilter = 0
    flags.Tile.correctTime = False
    flags.Tile.correctTimeJumps = False
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.doOverflowFit = False

    flags.Tile.RunType = TileRunType.PHY
    flags.Beam.Type = BeamType.Collisions
    # Get beam energy from meta data (Tile TB setup: [GeV])
    beamEnergy = GetFileMD(flags.Input.Files).get("beam_energy", 100)
    flags.Beam.Energy = beamEnergy * GeV

    # Set up the Tile input files
    if not flags.Input.Files and args.run:
        flags.Input.Files = TileInputFiles.findFilesFromAgruments(args)
    if not flags.Input.Files:
        log.error('Input files must be provided! For example: --filesInput=file1,file2,... or --run RUNNUMBER')
        sys.exit(-1)

    # Set up the Tile run type using arguments if it was not set up via configuration flags
    if args.cis:
        flags.Tile.RunType = TileRunType.CIS
    elif args.mono_cis:
        flags.Tile.RunType = TileRunType.MONOCIS
    elif args.laser:
        flags.Tile.RunType = TileRunType.LAS
    elif args.pedestals:
        flags.Tile.RunType = TileRunType.PED
    elif args.physics:
        flags.Tile.RunType = TileRunType.PHY

    # Set up Tile reconstuction method
    flags.Tile.doOpt2 = args.opt2
    flags.Tile.doOptATLAS = args.opt_atlas
    flags.Tile.doFit = args.fit

    # =======>>> Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    runNumber = flags.Input.RunNumbers[0]
    if not flags.Output.HISTFileName:
        flags.Output.HISTFileName = f'{args.outputDirectory}/tiletbmon_{runNumber}{args.outputVersion}.root'

    if args.preExec:
        log.info(f'Executing preExec: {args.preExec}')
        exec(args.preExec)

    flags.lock()

    log.info('=====>>> FINAL CONFIG FLAGS SETTINGS FOLLOW:')
    flags.dump(pattern='Tile.*|Beam.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    # Configure number of samples
    nSamples = args.nsamples
    if not nSamples:
        nSamples = 7 if flags.Tile.RunType.isBiGain() or runNumber <= 2110820 else 15
        log.info(f'Auto configure number of samples: {nSamples}')

    # =======>>> Initialize configuration object, add accumulator, merge, and run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # =======>>> Configure Tile raw data (digits) reading
    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                     readDigits=True,
                                     readRawChannel=True,
                                     readDigitsFlx=True,
                                     readBeamElem=True) )

    # =======>>> Configure reconstruction of Tile TestBeam data
    from TileTBRec.TileTestBeamRecoConfig import TileTestBeamRawChannelMakerCfg
    cfg.merge( TileTestBeamRawChannelMakerCfg(flags, nsamples=nSamples) )
    cfg.merge( TileTestBeamRawChannelMakerCfg(flags, nsamples=16, useFELIX=True) )

    if args.useSqlite:
        cfg.getService('IOVDbSvc').overrideTags += [
            f'<prefix>/TILE</prefix> <db>sqlite://;schema={args.useSqlite};dbname={flags.IOVDb.DatabaseInstance}</db>',
            # ROD folder does not exist in Sqlite file at the moment (should be added)
            f'<prefix>/TILE/ONL01/STATUS/ROD</prefix> <db>COOLONL_TILE/{flags.IOVDb.DatabaseInstance}</db>'
        ]

    # =======>>> Configure Tile TestBeam monitoring
    if args.mon:
        from TileTBRec.TileTestBeamRecoConfig import TileTestBeamRecoCfg
        cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=nSamples) )
        cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=16, useFELIX=True) )

        from TileMonitoring.RunTileTBMonitoring import TileTestBeamMonitoringCfg
        cfg.merge(TileTestBeamMonitoringCfg(flags, fragIDs=fragIDs, useFELIX=True))

    # =======>>> Configure Tile TestBeam h1000 Ntuple
    ntupleFile = f'{args.outputDirectory}/tiletb_{runNumber}{args.outputVersion}.aan.root'
    offlineUnits = args.offline_units
    if not offlineUnits:
        if flags.Tile.RunType in [TileRunType.PED, TileRunType.CIS]:
            offlineUnits = 0  # ADC
        elif flags.Tile.RunType in [TileRunType.LAS, TileRunType.MONOCIS]:
            offlineUnits = 1  # pC
        else:
            offlineUnits = 3  # MeV
        unitsName = {0: "ADC", 1: "pC", 2: "Cesium pC", 3: "MeV"}.get(offlineUnits)
        log.info(f'Auto configure offline units: {offlineUnits} => {unitsName}')

    calibrateEnergy = (offlineUnits != 0)

    from TileTBRec.TileTBAANtupleConfig import TileTBAANtupleCfg
    cfg.merge( TileTBAANtupleCfg(flags, outputFile=ntupleFile, OfflineUnits=offlineUnits, CalibrateEnergy=calibrateEnergy, NSamples=nSamples) )

    # =======>>> Configure Tile CIS calibration
    if flags.Tile.RunType in [TileRunType.CIS, TileRunType.MONOCIS]:
        cisCalibFile = f'{args.outputDirectory}/tileCalibCIS_{runNumber}_CIS{args.outputVersion}.root'
        from TileCalibAlgs.TileCisCalibAlgConfig import TileCisCalibAlgCfg
        cfg.merge( TileCisCalibAlgCfg(flags, FileName=cisCalibFile) )

        tileCisTool = cfg.getEventAlgo('TileCisCalibAlg').TileCalibTools['TileCisDefaultCalibTool']
        if runNumber >= 2200000:
            tileCisTool.FragIDsDemonstrators = [ 0x201, 0x402 ]
        elif runNumber >= 2100000:
            tileCisTool.FragIDsDemonstrators = [ 0x201 ]

    # =======>>> Configure Tile noise calibration
    if flags.Tile.RunType is TileRunType.PED:
        from TileCalibAlgs.TileDigiNoiseCalibAlgConfig import TileDigiNoiseCalibAlgCfg
        cfg.merge( TileDigiNoiseCalibAlgCfg(flags) )

    # =======>>> Configure ROD to ROB mapping
    # Scan first event for all fragments to create proper ROD to ROB map
    cfg.getCondAlgo('TileHid2RESrcIDCondAlg').RODStatusProxy = None

    # =======>>> Process post includes
    if args.postInclude:
        log.info(f'Process postInclude: {args.postInclude}')
        from PyJobTransforms.TransformUtils import processPostInclude
        processPostInclude(args, flags, cfg)

    # =======>>> Any last things to do?
    if args.postExec:
        log.info(f'Executing postExec: {args.postExec}')
        exec(args.postExec)

    if args.printConfig:
        cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

    if args.config_only:
        cfg.store(open('TileTestBeamMonitoring.pkl', 'wb'))
    else:
        sc = cfg.run()
        # Success should be 0
        sys.exit(not sc.isSuccess())
