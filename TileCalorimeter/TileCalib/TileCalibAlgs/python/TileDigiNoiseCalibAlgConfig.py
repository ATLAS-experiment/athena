#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileDigiNoiseCalibAlgConfig.py
@brief Python configuration of TileDigiNoiseCalibAlg algorithm for the Run III
'''
def TileDigiNoiseCalibAlgCfg(flags, **kwargs):

    ''' Function to configure TileDigiNoiseCalibAlg algorithm'''

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

    kwargs.setdefault('name', 'TileDigiNoiseCalibAlg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileDigiNoiseCalibAlgCfg = CompFactory.TileDigiNoiseCalibAlg

    acc.addEventAlgo(TileDigiNoiseCalibAlgCfg(**kwargs), primary=True)

    return acc

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readBeamElem=True) )

    cfg.merge( TileDigiNoiseCalibAlgCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileDigiNoiseCalibAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
