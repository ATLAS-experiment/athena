#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

'''
@file TileDigitsToNtupleConfig.py
@brief Python configuration of Tile digits to ntuple algorithm for the Run III
'''

from AthenaConfiguration.ComponentFactory import CompFactory


def TileDigitsToNtupleCfg(flags, outputFile=None, **kwargs):
    ''' Function to configure Tile digits to h40 ntuple algorithm.'''

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    if not outputFile:
        prefix = 'tiletb' if flags.Beam.Type is BeamType.TestBeam else 'tile'
        outputFile = f'{prefix}.ntup.root'

    ntupleSvc = CompFactory.NTupleSvc()
    ntupleSvc.Output = ["NTUP DATAFILE='%s' OPT='NEW'" % outputFile]
    acc.addService(ntupleSvc)

    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt')
    kwargs.setdefault('NTupleLoc', '/NTUP')

    TileDigitsToNtuple = CompFactory.TileDigitsToNtuple
    acc.addEventAlgo(TileDigitsToNtuple(**kwargs), primary=True)

    acc.setAppProperty('HistogramPersistency', 'ROOT')

    return acc


if __name__ == '__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN3
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()

    log.info('Final configuration flags follow:')
    flags.dump()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(TileDigitsToNtupleCfg(flags, TileDigitsContainer='TileDigitsFlt'))

    cfg.printConfig(withDetails=True, summariseProps=True)

    with open('TileDigitsToNtupleConfig.pkl', 'wb') as f:
        cfg.store(f)

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
