#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

'''
@file TileHitVecToNtupleConfig.py
@brief Python configuration of Tile hit vector to ntuple algorithm for the Run III
'''

from AthenaConfiguration.ComponentFactory import CompFactory


def TileHitVecToNtupleCfg(flags, outputFile=None, **kwargs):
    ''' Function to configure Tile hit vector to h32 ntuple algorithm.'''

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if not outputFile:
        outputFile = 'tiletb.ntup.root'

    ntupleSvc = CompFactory.NTupleSvc()
    ntupleSvc.Output = ["NTUP DATAFILE='%s' OPT='NEW'" % outputFile]
    acc.addService(ntupleSvc)

    kwargs.setdefault('MaxLength', 99999)
    kwargs.setdefault('TileHitVector', 'TileHitVec')
    kwargs.setdefault('NTupleLoc', '/NTUP')

    TileHitToNtuple = CompFactory.TileHitVecToNtuple
    acc.addEventAlgo(TileHitToNtuple(**kwargs), primary=True)

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
    flags.Input.Files = defaultTestFiles.HITS_RUN3
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    flags.dump()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(TileHitVecToNtupleCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store( open('TileHitVecToNtupleConfig.pkl', 'wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
