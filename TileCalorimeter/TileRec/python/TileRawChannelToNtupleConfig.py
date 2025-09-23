#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

'''
@file TileRawChannelToNtupleConfig.py
@brief Python configuration of Tile raw channels to ntuple algorithm for the Run III
'''

from AthenaConfiguration.ComponentFactory import CompFactory


def TileRawChannelToNtupleCfg(flags, outputFile=None, **kwargs):
    ''' Function to configure Tile digits to h70 ntuple algorithm.'''

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if not outputFile:
        prefix = 'tiletb' if flags.Beam.Type is BeamType.TestBeam else 'tile'
        outputFile = f'{prefix}.ntup.root'

    ntupleSvc = CompFactory.NTupleSvc()
    ntupleSvc.Output = ["NTUP DATAFILE='%s' OPT='NEW'" % outputFile]
    acc.addService(ntupleSvc)

    kwargs.setdefault('TileRawChannelContainer', 'TileRawChannelCnt')
    kwargs.setdefault('NTupleLoc', '/NTUP')

    TileRawChannelToNtuple = CompFactory.TileRawChannelToNtuple
    acc.addEventAlgo(TileRawChannelToNtuple(**kwargs), primary=True)

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

    cfg.merge(TileRawChannelToNtupleCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)

    with open('TileRawChannelToNtupleConfig.pkl', 'wb') as f:
        cfg.store(f)

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)
