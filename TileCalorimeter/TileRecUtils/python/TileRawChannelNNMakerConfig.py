# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile NN raw channel maker algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TileRawChannelNNMakerCfg(flags, **kwargs):
    """Return component accumulator with configured Tile NN raw channel maker algorithm

    Arguments:
        flags  -- Athena configuration flags
    """
    acc = ComponentAccumulator()
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg

    acc.merge(DetDescrCnvSvcCfg(flags))
    TileRawChannelNNMaker = CompFactory.TileRawChannelNNMaker
    acc.addEventAlgo(TileRawChannelNNMaker(**kwargs), primary=True)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    from TileConfiguration.TileConfigFlags import TileRunType

    log.setLevel(DEBUG)
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    acc = MainServicesCfg(flags)
    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg

    acc.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )
    acc.merge( TileRawChannelNNMakerCfg(flags) )
    flags.dump()
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store( open('TileRawChannelNNMaker.pkl', 'wb') )
    sc = acc.run(maxEvents=3)
    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
