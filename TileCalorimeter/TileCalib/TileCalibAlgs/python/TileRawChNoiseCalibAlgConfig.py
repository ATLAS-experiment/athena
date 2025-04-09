#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileRawChNoiseCalibAlgConfig.py
@brief Python configuration of TileRawChNoiseCalibAlg algorithm for the Run III
'''
def TileRawChNoiseCalibAlgCfg(flags, **kwargs):

    ''' Function to configure TileRawChNoiseCalibAlg algorithm'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg( flags ))

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    if 'TileBadChanTool' not in kwargs:
        from TileConditions.TileBadChannelsConfig import TileBadChanToolCfg
        badChanTool = acc.popToolsAndMerge( TileBadChanToolCfg(flags) )
        kwargs['TileBadChanTool'] = badChanTool

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    kwargs.setdefault('name', 'TileRawChNoiseCalibAlg')
    kwargs.setdefault('doDsp', True)
    kwargs.setdefault('doFit', flags.Tile.doFit)
    kwargs.setdefault('doOpt', flags.Tile.doOpt2)
    kwargs.setdefault('doFixed', flags.Tile.doOptATLAS)
    kwargs.setdefault('doMF', flags.Tile.doMF)
    kwargs.setdefault('doOF1', flags.Tile.doOF1)

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileRawChNoiseCalibAlg = CompFactory.TileRawChNoiseCalibAlg

    acc.addEventAlgo(TileRawChNoiseCalibAlg(**kwargs), primary=True)

    return acc

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readBeamElem=True) )

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )

    cfg.merge( TileRawChNoiseCalibAlgCfg(flags, ) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileRawChNoiseCalibAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
