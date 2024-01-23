#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''@file RunTileTBDump.py
@brief Script to run Tile Reconstrcution/Monitoring for calibration runs
'''

from TileRecEx import TileInputFiles
from AthenaConfiguration.Enums import Format


epiLog = """
Examples:

    RunTileTBDump.py --run RUNNUMBER --evtMax 1
    RunTileTBDump.py --filesInput=FILE1,FILE2 Exec.SkipEvents=100

At least one should provide the following arguments or Athena configuration flags (flags have higher priority):
   Input file(s), e.g.: --run RUNNUMBER | --filesInput=FILE1,FILE2 | Input.Files="['FILE1','FILE2']"
"""


if __name__=='__main__':
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    parserParents = [flags.getArgumentParser(), TileInputFiles.getArgumentParser(add_help=False)]

    import argparse
    parser = argparse.ArgumentParser(parents=parserParents, add_help=False, epilog=epiLog, formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--printDetailedConfig', action='store_true', help='Print detailed Athena configuration')

    algorithms_group = parser.add_argument_group('TileTBDump and/or TileTBStat aglorithms to run')
    algorithms_group.add_argument('--stat', action='store_true', help='Run TileTBstat algorithm')
    algorithms_group.add_argument('--no-dump', action='store_false', dest='dump',  help='Do not run TileTBDump algorithm')

    parser.add_argument('--dump-bs-fragments', action='store_true', dest='dump_bs_fragments', help='Dump fragments from ByteStreamInputSvc')

    run_period_group = parser.add_argument_group('LHC Run period')
    run_period = run_period_group.add_mutually_exclusive_group()
    run_period.add_argument('--run2', action='store_true', help='LHC Run2 period')
    run_period.add_argument('--run3', action='store_true', help='LHC Run3 period')

    args, _ = parser.parse_known_args()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    log.setLevel(Constants.INFO)

    # Initially the following flags are not set up (they must be provided)
    flags.Input.Files = []

    # Initial configuration flags from command line arguments (to be used to set up defaults)
    flags.fillFromArgs(parser=parser)

    # =======>>> Set up the Athena configuration flags to defaults (can be overriden via comand line)

    # Set up the Tile input files
    if not flags.Input.Files and args.run:
        flags.Input.Files = TileInputFiles.findFilesFromAgruments(args)
    if not flags.Input.Files:
        log.error('Input files must be provided! For example: --filesInput=file1,file2,... or --run RUNNUMBER')
        sys.exit(-1)

    runNumber = flags.Input.RunNumbers[0]

    # Set up LHC Run period
    if not any([args.run2, args.run3]):
        if not flags.Input.isMC:
            if runNumber >= 411938 or runNumber <= 0 or runNumber == 3:
                args.run3 = True
            elif runNumber > 232000 or runNumber == 2:
                args.run2 = True

    # Set up the DB global conditions tag
    if flags.Input.Format is Format.BS:
        if args.run3 or flags.Input.isMC:
            condDbTag = 'OFLCOND-MC23-SDR-RUN3-01' if flags.Input.isMC else 'CONDBR2-BLKPA-2023-01'
            detDescrVersion = 'ATLAS-R3S-2021-03-01-00'
        elif args.run2:
            condDbTag = 'CONDBR2-BLKPA-2018-16'
            detDescrVersion = 'ATLAS-R2-2016-01-00-01'
        else:
            condDbTag = 'COMCOND-BLKPA-RUN1-06'
            detDescrVersion = 'ATLAS-R1-2012-03-02-00'

        flags.IOVDb.GlobalTag = condDbTag
        flags.GeoModel.AtlasVersion = detDescrVersion


    # Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    flags.needFlagsCategory('Tile')

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    flags.lock()

    log.info('=====>>> FINAL CONFIG FLAGS SETTINGS FOLLOW:')
    flags.dump(pattern='Tile.*|Input.*|Exec.*|IOVDb.[D|G].*', evaluate=True)

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # =======>>> Set up the File (BS | POOL) reading
    if flags.Input.Format is Format.BS:
        # Configure reading the Tile BS files
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readBeamElem=args.stat, readLaserObj=args.stat) )

        from AthenaCommon.Constants import VERBOSE
        if flags.Exec.OutputLevel is VERBOSE:
            for alg in ['TileDigitsReadAlg', 'TileRawChannelReadAlg']:
                cfg.getEventAlgo(alg).TileROD_Decoder.VerboseOutput = True

        if args.dump_bs_fragments:
            cfg.getService('ByteStreamInputSvc').DumpFlag = True

    else:
        log.error('Input files must be in BS format')
        sys.exit(-1)

    if args.dump:
        dump_once = True if args.stat else False
        from TileTBRec.TileTBDumpConfig import TileTBDumpCfg
        cfg.merge(TileTBDumpCfg(flags, dumpOnce=dump_once))

    if args.stat:
        from TileTBRec.TileTBStatConfig import TileTBStatCfg
        cfg.merge(TileTBStatCfg(flags))

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=args.printDetailedConfig)

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
