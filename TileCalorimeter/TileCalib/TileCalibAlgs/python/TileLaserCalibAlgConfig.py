#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TileConfiguration.TileConfigFlags import TileRunType

'''
@file TileLaserCalibAlgConfig.py
@brief Python configuration of TileLaserDefaultCalibTool tool for the Run III
'''
def TileLaserDefaulCalibToolCfg(flags, **kwargs):

    ''' Function to configure TileLaserDefaultCalibTool tool'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

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

    if 'TileDCSTool' not in kwargs:
        from TileConditions.TileDCSConfig import TileDCSToolCfg
        kwargs['TileDCSTool'] = acc.popToolsAndMerge( TileDCSToolCfg(flags) )

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileLaserDefalutCalibTool = CompFactory.TileLaserDefaultCalibTool

    acc.setPrivateTools(TileLaserDefalutCalibTool(**kwargs))

    return acc


'''
@brief Python configuration of TileLaserCalibAlg algorithm for the Run III
'''
def TileLaserCalibAlgCfg(flags, **kwargs):

    ''' Function to configure TileLaserCalibAlg algorithm'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    if 'Tools' not in kwargs:
        laserCalibTool = acc.popToolsAndMerge( TileLaserDefaulCalibToolCfg(flags) )
        kwargs['Tools'] = [laserCalibTool]

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileLaserCalibAlg = CompFactory.TileLaserCalibAlg

    acc.addEventAlgo(TileLaserCalibAlg(**kwargs), primary=True)

    return acc

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    inputDirectory = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileByteStream/TileByteStream-02-00-00'
    inputFile = 'data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data'

    flags = initConfigFlags()
    flags.Input.Files = [inputDirectory + '/' + inputFile]
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Tile.RunType = TileRunType.LAS
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags) )

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )

    cfg.merge( TileLaserCalibAlgCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileLaserCalibAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
