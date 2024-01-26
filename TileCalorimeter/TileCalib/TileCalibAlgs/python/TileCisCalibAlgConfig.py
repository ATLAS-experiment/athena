#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TileConfiguration.TileConfigFlags import TileRunType

'''
@file TileCisCalibAlgConfig.py
@brief Python configuration of TileCisDefaultCalibTool tool for the Run III
'''
def TileCisDefaulCalibToolCfg(flags, **kwargs):

    ''' Function to configure TileCisDefaultCalibTool tool'''

    kwargs.setdefault('removePed', True)

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

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileCisDefalutCalibTool = CompFactory.TileCisDefaultCalibTool

    acc.setPrivateTools(TileCisDefalutCalibTool(**kwargs))

    return acc


'''
@brief Python configuration of TileCisCalibAlg algorithm for the Run III
'''
def TileCisCalibAlgCfg(flags, **kwargs):

    ''' Function to configure TileCisCalibAlg algorithm'''

    kwargs.setdefault('name', 'TileCisCalibAlg')
    kwargs.setdefault('RunType', TileRunType.CIS.getIntValue())
    kwargs.setdefault('RunNumber', flags.Input.RunNumbers[0])

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    if 'TileCalibTools' not in kwargs:
        cisCalibTool = acc.popToolsAndMerge( TileCisDefaulCalibToolCfg(flags) )
        kwargs['TileCalibTools'] = [cisCalibTool]

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileTopCalibAlg = CompFactory.TileTopCalibAlg

    acc.addEventAlgo(TileTopCalibAlg(**kwargs), primary=True)

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
    flags.Tile.doFit = True
    flags.Tile.RunType = TileRunType.CIS
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

    cfg.merge( TileCisCalibAlgCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileCisCalibAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
