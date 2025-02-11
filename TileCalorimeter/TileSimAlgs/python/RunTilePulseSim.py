#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
'''@file RunTilePulseSim.py
@brief Script to run Tile pulse simulator
'''

from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType


def TileDigitsFromPulseCfg(flags, **kwargs):
    """Return component accumulator with configured Tile digits from pulse algorithm

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        OutputDigitsContainer         -- Name of container with TileDigits to write
        ImperfectionMean              -- Mean value of pulse shape broadening
        ImperfectionRMS               -- RMS of pulse shape broadening
        InTimeAmp                     -- Amplitude of in-time pulse
        OutOfTimeAmp                  -- Amplitude of out-of-time pulse
        InTimeOffset                  -- In-time pulse offset from nominal time
        OutOfTimeOffset               -- Out-of-time pulse offset from nominal time
        UseGaussNoise                 -- Set to TRUE in order to create noise from double gaussian
        GaussNoiseAmpOne              -- Amplitude of first gaussian of double gaussian noise
        GaussNoiseSigmaOne            -- Standard deviation of first gaussian of double gaussian noise
        GaussNoiseAmpTwo              -- Amplitude of second gaussian of double gaussian noise
        GaussNoiseSigmaTwo            -- Standard deviation of second gaussian of double gaussian noise
        UseInTimeAmpDist              -- Set to TRUE in order to use a distribution for the in-time amplitude instead of a const.
        UseOutOfTimeAmpDist           -- Set to TRUE in order to use a distribution for the out-of-time amplitude instead of a const
        InTimeAmpDistFileName         -- Filename of file to use for amplitude distribution of in-time pulses
        OutOfTimeAmpDistFileName      -- Filename of file to use for amplitude distribution of out-of-time pulses
        PileUpFraction                -- Probability that an out-of-time component will be added
        GaussianC2CPhaseVariation     -- RMS for the in-time pulse offset (channel-to-channel phase variation)
        ChannelSpecificPedestal       -- Set to TRUE in order to use a channel specific value for the pedestal
        ChannelSpecificNoise          -- Set to TRUE in order to add channel specific noise
        OutOfTimeOffsetHistogramFile  -- Filename of file containing histogram of pile-up timing distribution
        OutOfTimeOffsetHistogramName  -- Name of the histogram to use for pile-up timing distribution
        AmpDistLowerLimit             -- Set all bins lower than this to zero. Default = 135
        InTimeAmpDistHistogramName    -- Name of the histogram to use for in-time amplitude distribution
        OutOfTimeAmpDistHistogramName -- Name of the histogram to use for out-of-time amplitude distribution
    """

    kwargs.setdefault('InTimeAmp', 1000)
    kwargs.setdefault('ImperfectionMean', 1)
    kwargs.setdefault('ImperfectionRms', 0)
    kwargs.setdefault('TilePhaseII', False)
    kwargs.setdefault('NSamples', 7)

    PhaseII = kwargs['TilePhaseII']
    # PhaseII parameters
    if PhaseII:
        kwargs.setdefault('PedestalValueHG', 100)
        kwargs.setdefault('PedestalValueLG', 100)
        kwargs.setdefault('ChannelSpecificPedestal', False)
        kwargs.setdefault('UseGaussNoise', True)
    else:
        kwargs.setdefault('ChannelSpecificPedestal', True)
        kwargs.setdefault('ChannelSpecificNoise', True)

    kwargs.setdefault('PileUpFraction', 0)
    kwargs.setdefault('AmpDistLowerLimit', 0)
    kwargs.setdefault('AvgMuForPileUpSimulation', 80)

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc = TileGMCfg(flags)

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileSampleNoiseConfig import TileSampleNoiseCondAlgCfg
    acc.merge( TileSampleNoiseCondAlgCfg(flags) )

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs['RndmSvc'] = acc.getPrimaryAndMerge( AthRNGSvcCfg(flags) ).name

    # Configure TileInfoLoader to set up number of samples
    nSamples = kwargs['NSamples']
    ADCmax = 4095 if PhaseII else 1023
    ADCmaskValue = 4800 if PhaseII else 2047
    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags,
                                 NSamples=nSamples, TrigSample=((nSamples-1)//2),
                                 ADCmax=ADCmax, ADCmaskValue=ADCmaskValue) )

    TileDigitsFromPulse = CompFactory.TileDigitsFromPulse
    acc.addEventAlgo(TileDigitsFromPulse(**kwargs), primary=True)

    return acc


if __name__ == '__main__':
    import sys

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    log.setLevel(Constants.INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    parser = flags.getArgumentParser(description='Run Tile pulse simulator. \
    One can use it without input file and default conditions will be used. \
    Or one can use --run to specify run number from which conditions should be used \
    (probably conditions and geometries tags should be changed also in this case). \
    Or one can use --filesInput to specify input file with HITS to take conditions from. \
    Example: athena --CA RunTilePulseSim.py --evtMax 10')

    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--printDetailedConfig', action='store_true', help='Print detailed Athena configuration')
    parser.add_argument('--outputDirectory', default='.', help='Output directory for produced files')
    parser.add_argument('--outputVersion', type=str, default='0', help='Version to be used in output file for ntuple')
    parser.add_argument('--nsamples', type=int, default=7, help='Number of samples')
    parser.add_argument('--phaseII', type=bool, default=False, help='Use parameters of TilePhaseII')
    parser.add_argument('--run', type=int, default=410000, help='Run number')
    parser.add_argument('--save-true-amplitude', action='store_true', help='Save true Tile raw channel amplitude into h2000')
    parser.add_argument('--acr-db', action='store_true', help='Use auto correlation matrix from DB')

    args, _ = parser.parse_known_args()

    flags.Tile.RunType = TileRunType.PHY
    # Set up Tile reconstuction methods
    flags.Tile.doOpt2 = True
    flags.Tile.doOptATLAS = True
    if args.nsamples != 7:
        flags.Tile.OfcFromCOOL = False

    flags.Input.isMC = True
    flags.Input.Files = []
    flags.Exec.MaxEvents = 3

    # Override default configuration flags from command line arguments
    flags.fillFromArgs(parser=parser)

    if not flags.Input.Files:
        flags.Input.RunNumbers = [args.run]
        flags.Input.ConditionsRunNumber = args.run
        flags.Input.OverrideRunNumber = True

        from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC

    if args.preExec:
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    flags.lock()
    flags.dump()

    runNumber = flags.Input.RunNumbers[0]

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    if flags.Input.Files:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    # =======>>> Configure Tile digits from pulse algorithm
    cfg.merge( TileDigitsFromPulseCfg(flags, NSamples=args.nsamples, TilePhaseII=args.phaseII) )

    # =======>>> Configure Tile raw channel maker
    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )
    if not flags.Tile.OfcFromCOOL:
        rawChannelBuilders = cfg.getEventAlgo('TileRChMaker').TileRawChannelBuilder
        for rawChannelBuilder in rawChannelBuilders:
            if hasattr(rawChannelBuilder, 'TileCondToolOfc'):
                rawChannelBuilder.TileCondToolOfc.nSamples = args.nsamples
                # True - unity matrix, False - use matrix from DB
                rawChannelBuilder.TileCondToolOfc.OptFilterDeltaCorrelation = not args.acr_db

    # =======>>> Configure Tile h2000 ntuple production
    ntupleFile = f'{args.outputDirectory}/tile_{runNumber}_{args.outputVersion}.aan.root'
    from TileRec.TileAANtupleConfig import TileAANtupleCfg
    cfg.merge( TileAANtupleCfg(flags,
                               saveTMDB=False,
                               NSamples=args.nsamples,
                               TileL2Cnt='',
                               TileDigitsContainerFlt='',
                               TileDigitsContainer='TileDigitsCnt',
                               TileRawChannelContainer='TileRawChannelCnt',
                               TileRawChannelContainerOpt='TileRawChannelOpt2',
                               CalibrateEnergy=False,
                               OfflineUnits=0,
                               CalibMode=True,
                               outputFile=ntupleFile) )
    if args.save_true_amplitude:
        cfg.getEventAlgo('TileNtuple').TileRawChannelContainerFit = 'TrueAmp'

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=args.printDetailedConfig,
                    summariseProps=args.printDetailedConfig,
                    printDefaults=args.printDetailedConfig)

    if not args.config_only:
        sc = cfg.run()
        sys.exit(0 if sc.isSuccess() else 1)
