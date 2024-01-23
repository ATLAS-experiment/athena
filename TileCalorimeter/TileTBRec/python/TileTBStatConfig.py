#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


'''
@file TileTBStatConfig.py
@brief Python configuration of TileTBStat algorithm for the Run III
'''


def TileTBStatCfg(flags, **kwargs):
    ''' Function to configure TileTBStat algorithm.'''

    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags, TileBeamElemContainer='TileBeamElemCnt') )

    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    acc.merge( TileHid2RESrcIDCondAlgCfg(flags, ROD2ROBmap=['-1']) )

    acc.addService(CompFactory.ROBDataProviderSvc())

    TileTBStat = CompFactory.TileTBStat
    acc.addEventAlgo(TileTBStat(**kwargs), primary = True)

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

    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs(parser=parser)

    flags.lock()

    flags.dump()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readBeamElem=True) )

    cfg.merge( TileTBStatCfg(flags) )

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
