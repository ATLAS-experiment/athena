
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


'''
@file TileTBDumpConfig.py
@brief Python configuration of TileTBDump algorithm for the Run III
'''


def TileTBDumpCfg(flags, **kwargs):
    ''' Function to configure TileTBDump algorithm.'''

    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if 'TileCondToolTiming' not in kwargs:
        from TileConditions.TileTimingConfig import TileCondToolTimingCfg
        kwargs['TileCondToolTiming'] = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    if 'TileCondToolOfcCool' not in kwargs:
        from TileConditions.TileOFCConfig import TileCondToolOfcCoolCfg
        kwargs['TileCondToolOfcCool'] = acc.popToolsAndMerge( TileCondToolOfcCoolCfg(flags) )

    acc.addService(CompFactory.ROBDataProviderSvc())

    TileTBDump = CompFactory.TileTBDump
    acc.addEventAlgo(TileTBDump(**kwargs), primary = True)

    return acc


if __name__=='__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
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

    flags.lock()

    flags.dump()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge( ByteStreamReadCfg(flags) )

    cfg.merge( TileTBDumpCfg(flags) )

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
