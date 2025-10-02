#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from TileConfiguration.TileConfigFlags import TileRunType

'''
@file TileAANtupleConfig.py
@brief Python configuration of TileAANtuple algorithm for the Run III
'''
def TileAANtupleCfg(flags, outputFile='', saveTMDB=True, **kwargs):
    ''' Function to configure TileAANtuple algorithm.'''


    cisRun      = flags.Tile.RunType is TileRunType.CIS
    laserRun    = flags.Tile.RunType is TileRunType.LAS
    pedestalRun = flags.Tile.RunType is TileRunType.PED
    physicsRun  = flags.Tile.RunType is TileRunType.PHY

    readDigits = flags.Tile.readDigits

    kwargs.setdefault('UseDspUnits', False)
    useDspUnits = kwargs['UseDspUnits']

    offlineUnits = 3  # MeV (default)
    if useDspUnits:
        offlineUnits = -1
    elif cisRun:
        offlineUnits = 0  # ADC
    elif not (pedestalRun or physicsRun):
        offlineUnits = 1  # pCb

    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt' if readDigits else "")
    kwargs.setdefault('TileDigitsContainerFlt', 'TileDigitsFlt' if not readDigits else "")
    kwargs.setdefault('TileRawChannelContainer', flags.Tile.RawChannelContainer)
    kwargs.setdefault('TileRawChannelContainerFit', 'TileRawChannelFit' if flags.Tile.doFit else "")
    kwargs.setdefault('TileRawChannelContainerFitCool', 'TileRawChannelFitCool' if flags.Tile.doFitCOOL else "")
    kwargs.setdefault('TileRawChannelContainerOpt', "")
    kwargs.setdefault('TileRawChannelContainerQIE', 'TileRawChannelQIE' if flags.Tile.doQIE else "")
    kwargs.setdefault('TileRawChannelContainerOF1', 'TileRawChannelOF1' if flags.Tile.doOF1 else "")
    kwargs.setdefault('TileRawChannelContainerMF', 'TileRawChannelMF' if flags.Tile.doMF else "")
    kwargs.setdefault('TileRawChannelContainerWiener', "")
    kwargs.setdefault('TileRawChannelContainerDsp', "")
    kwargs.setdefault('TileLaserObject', 'TileLaserObj' if laserRun else "")
    kwargs.setdefault('TileBeamElemContainer', 'TileBeamElemCnt' if not physicsRun else "")

    kwargs.setdefault('TileMuRcvRawChannelContainer', 'MuRcvRawChCnt' if saveTMDB else "")
    kwargs.setdefault('TileMuRcvDigitsContainer', 'MuRcvDigitsCnt' if saveTMDB else "")
    kwargs.setdefault('TileMuRcvContainer', "TileMuRcvCnt" if saveTMDB else "")

    kwargs.setdefault('CheckDCS', flags.Tile.useDCS)
    kwargs.setdefault('BSInput', flags.Input.Format is Format.BS and not physicsRun)
    kwargs.setdefault('CalibMode', pedestalRun or cisRun)
    kwargs.setdefault('CalibrateEnergy', flags.Input.isMC or useDspUnits or not cisRun)
    kwargs.setdefault('OfflineUnits', offlineUnits)

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if 'TileBadChanTool' not in kwargs:
        from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
        acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    if kwargs['CheckDCS']:
        from TileConditions.TileDCSConfig import TileDCSCondAlgCfg
        acc.merge( TileDCSCondAlgCfg(flags) )

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    if not outputFile:
        run = str(flags.Input.RunNumbers[0])
        outputFile = 'tile_{}.aan.root'.format(run)
    histsvc = CompFactory.THistSvc()
    histsvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % ('AANT', outputFile)]
    acc.addService(histsvc)

    TileAANtuple = CompFactory.TileAANtuple
    acc.addEventAlgo(TileAANtuple('TileNtuple', **kwargs), primary = True)

    return acc


if __name__=='__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--no-tmdb', dest='tmdb', action='store_false', help='Do not save TMDB information into ntuple')
    args, _ = parser.parse_known_args()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs(parser=parser)

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    flags.dump()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    rawChannelContainer = 'TileRawChannelCnt'

    if flags.Input.Format is Format.BS:
        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        cfg.merge( TileRawDataReadingCfg(flags,
                                         readMuRcv=args.tmdb,
                                         readMuRcvDigits=args.tmdb,
                                         readMuRcvRawCh=args.tmdb) )

    else:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

        inputCollections = flags.Input.Collections
        if rawChannelContainer not in inputCollections:
            rawChannelContainer = 'TileRawChannelFlt' if 'TileRawChannelFlt' in inputCollections else ""

    cfg.merge( TileAANtupleCfg(flags, TileRawChannelContainer=rawChannelContainer, saveTMDB = args.tmdb) )

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails = True, summariseProps = True)

    cfg.store( open('TileAANtuple.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
