# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured TileRawChannelToHit algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileRawChannelToHitCfg(flags, **kwargs):
    """Return component accumulator with configured TileRawChannelToHit algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('UseSamplFract', False)

    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if kwargs['UseSamplFract']:
        from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
        acc.merge( TileSamplingFractionCondAlgCfg(flags) )

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    TileRawChannelToHit = CompFactory.TileRawChannelToHit
    acc.addEventAlgo(TileRawChannelToHit(**kwargs), primary=True)

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO

    # Test setup
    log.setLevel(INFO)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = 'CONDBR2-ES1PA-2018-02'
    flags.Tile.RunType = TileRunType.PHY
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readDigits=False) )

    cfg.merge( TileRawChannelToHitCfg(flags) )

    flags.dump()
    cfg.printConfig(withDetails=True, summariseProps=True)
    cfg.store( open('TileRawChannelToHit.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

