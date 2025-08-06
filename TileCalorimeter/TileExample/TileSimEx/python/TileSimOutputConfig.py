#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def TileSimOutputCfg(flags, ntupleOutput=None, d3pdOutput=None):
    ''' Function to configure Tile ntuples and D3PD for simulaiton.'''

    acc = ComponentAccumulator()

    if ntupleOutput is None:
        ntupleOutput='tiletb.ntup.root'
    if d3pdOutput is None:
        d3pdOutput='tiletb.d3pd.root'

    if ntupleOutput:
        from TileRec.TileHitVecToNtupleConfig import TileHitVecToNtupleCfg
        acc.merge(TileHitVecToNtupleCfg(flags, outputFile=ntupleOutput))

    if d3pdOutput:
        from TileSimEx.TileSimD3PDConfig import TileSimD3PDCfg
        acc.merge(TileSimD3PDCfg(flags, outputFile=d3pdOutput))

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

    cfg.merge(TileSimOutputCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store( open('TileSimOutputConfig.pkl', 'wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
