# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured MBTS time difference algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def MBTSTimeDiffEventInfoAlgCfg(flags, **kwargs):
    """Return component accumulator with configured MBTS time difference algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    MBTSTimeDiffEventInfoAlg=CompFactory.MBTSTimeDiffEventInfoAlg
    acc.addEventAlgo(MBTSTimeDiffEventInfoAlg(**kwargs), primary = True)

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.AOD_RUN2_DATA
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge( MBTSTimeDiffEventInfoAlgCfg(flags) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('MBTSTimeDiffEventInfoAlg.pkl','wb') )

    sc = acc.run(maxEvents = 3)

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

