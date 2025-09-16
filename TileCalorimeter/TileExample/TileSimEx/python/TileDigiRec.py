"""
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.Enums import ProductionStep, BeamType
from TileConfiguration.TileConfigFlags import TileRunType
import sys


def main():

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log

    flags = initConfigFlags()

    parser = flags.getArgumentParser(description='Run Tile TB digitization.')
    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')

    parser.add_argument('--run-number', default=None, help='Override run number for conditions')
    parser.add_argument('--conditions-tag', default=None, help='Override conditions tag')
    parser.add_argument('--layout', default='5B', choices=['2B1EB', '2B2EB', '3B', '5B'], help='Tile TB layout')
    parser.add_argument('--no-calo-noise', action='store_true', help='Switch off Calo noise')
    parser.add_argument('--aant-ntuple', action='store_true', help='Produce output Tile (TB) AANT ntuple (h1000/h2000)')
    parser.add_argument('--hits-ntuple', action='store_true', help='Produce output ntuple with Tile hits')
    parser.add_argument('--d3pd', action='store_true', help='Produce output Tile D3PD file')
    parser.add_argument('--hits-d3pd', action='store_true', help='Save Tile hits into D3PD')
    parser.add_argument('--hits-info-d3pd', action='store_true', help='Save Tile hits info into D3PD')
    parser.add_argument('--digits-d3pd', action='store_true', help='Save Tile digits into D3PD')
    parser.add_argument('--channels-d3pd', action='store_true', help='Save Tile raw channels into D3PD')
    parser.add_argument('--cells-d3pd', action='store_true', help='Save Tile cells into D3PD')
    parser.add_argument('--cells-info-d3pd', action='store_true', help='Save Tile cells into D3PD')
    parser.add_argument('--mbts-d3pd', action='store_true', help='Save Tile MBTS into D3PD')
    parser.add_argument('--rdo', action='store_true', help='Produce output Tile RDO file')
    parser.add_argument('--file-prefix', default=None, help='Prefix to be used in names of output files')
    parser.add_argument('--sfr-tag', default=None, help='Override Tile sampling fraction tag')
    parser.add_argument('--testbeam', action='store_true', help='Digitize Test beam simulation')
    parser.add_argument('--trigger', action='store_true', help='Simulate Tile trigger output')
    parser.add_argument('--alldigits-rdo', action='store_true', help='Save all Tile digits into RDO')

    args, _ = parser.parse_known_args()

    flags.Input.Files = defaultTestFiles.HITS_RUN3
    flags.Exec.MaxEvents = 3

    flags.Tile.RunType = TileRunType.PHY
    flags.Tile.doFit = True
    flags.Tile.doOpt2 = True

    if args.testbeam:
        flags.Beam.Type = BeamType.TestBeam
        flags.TestBeam.Layout = f'tb_Tile2000_2003_{args.layout}'

    flags.Common.ProductionStep = ProductionStep.Digitization
    flags.Digitization.PileUp = False
    flags.Digitization.DoCaloNoise = not args.no_calo_noise

    if args.run_number:
        flags.Input.OverrideRunNumber = True
        flags.Input.ConditionsRunNumber = args.run_number

    if args.conditions_tag:
        flags.IOVDb.GlobalTag = args.conditions_tag

    filePrefix = ""
    if args.file_prefix:
        filePrefix = args.file_prefix
    else:
        prefix = 'tiletb' if args.testbeam else 'tile'
        filePrefix = f'{prefix}_{flags.Input.RunNumbers[0]}'

    if args.rdo:
        flags.Output.doWriteRDO = True
        outputRDO = f'{filePrefix}.RDO.pool.root'
        flags.Output.RDOFileName = outputRDO

    # Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    flags.lock()

    log.info('Final configuration flags follow:')
    flags.dump()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from TileSimAlgs.TileDigitizationConfig import TileDigitizationCfg
    cfg.merge(TileDigitizationCfg(flags))

    if args.trigger:
        from TileSimAlgs.TileDigitizationConfig import TileTriggerDigitizationCfg
        cfg.merge(TileTriggerDigitizationCfg(flags))

    if any([args.cells_info_d3pd, args.cells_d3pd, args.mbts_d3pd]):
        from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
        cfg.merge(TileCellMakerCfg(flags))
        cfg.getEventAlgo('TileCellMaker').CaloCellMakerToolNames['TileCellBuilder'].maskBadChannels = False

    if args.aant_ntuple:
        outputAANtuple = f'{filePrefix}.aant.root'
        if args.testbeam:
            from TileTBRec.TileTBAANtupleConfig import TileTBAANtupleCfg
            cfg.merge(TileTBAANtupleCfg(flags, outputFile=outputAANtuple))
        else:
            from TileRec.TileAANtupleConfig import TileAANtupleCfg
            cfg.merge(TileAANtupleCfg(flags, outputFile=outputAANtuple,
                                      TileL2Cnt='TileL2Cnt' if args.trigger else "",
                                      TileMuRcvContainer='TileMuRcvCnt' if args.trigger else "",
                                      TileMuRcvDigitsContainer='MuRcvDigitsCnt' if args.trigger else "",
                                      TileMuRcvRawChannelContainer='MuRcvRawChCnt' if args.trigger else ""))

    if args.hits_ntuple:
        outputHitsNtuple = f'{filePrefix}.ntup.root'
        from TileRec.TileHitVecToNtupleConfig import TileHitVecToNtupleCfg
        cfg.merge(TileHitVecToNtupleCfg(flags, outputFile=outputHitsNtuple))

    if args.d3pd:
        outputD3PD = f'{filePrefix}.d3pd.root'
        from TileSimEx.TileSimD3PDConfig import TileSimD3PDCfg
        cfg.merge(TileSimD3PDCfg(flags, outputFile=outputD3PD,
                                 saveHitsInfo=args.hits_info_d3pd,
                                 saveHits=args.hits_d3pd,
                                 saveDigits=args.digits_d3pd,
                                 saveChannels=args.channels_d3pd,
                                 saveCellsInfo=args.cells_info_d3pd,
                                 saveCells=args.cells_d3pd,
                                 saveMBTS=args.mbts_d3pd))

    if args.sfr_tag:
        from IOVDbSvc.IOVDbSvcConfig import addOverride
        cfg.merge(addOverride(flags, '/TILE/OFL02/CALIB/SFR', f'{args.sfr_tag}'))

    if args.rdo:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        outputItemList = []
        if args.alldigits_rdo:
            outputItemList += ['TileDigitsContainer#TileDigitsCnt']
        cfg.merge(OutputStreamCfg(flags, streamName='RDO', ItemList=outputItemList))

        # Add in-file MetaData
        from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
        cfg.merge(SetupMetaDataForStreamCfg(flags, "RDO"))

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

    if args.config_only:
        with open('TileDigiRec.pkl', 'wb') as f:
            cfg.store(f)
    else:
        sc = cfg.run()
        sys.exit(0 if sc.isSuccess() else 1)


if __name__ == "__main__":
    main()
