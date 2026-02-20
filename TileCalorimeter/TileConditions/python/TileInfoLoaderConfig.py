# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile info loader service"""


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

@AccumulatorCache
def TileInfoLoaderCfg(flags, **kwargs):
    """Return component accumulator with configured Tile info loader service

    Arguments:
        flags  -- Athena configuration flags
    """

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('TileInfoLoaderCfg')

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc = ComponentAccumulator()
    acc.merge (TileCablingSvcCfg(flags))

    kwargs.setdefault('LoadWienerFilterWeights', flags.Tile.doWiener)

    if 'NoiseScaleIndex' not in kwargs:
        if flags.Tile.doOpt2 and not flags.Tile.doOptATLAS:
            msg.info("Adjusting TileInfo to return cell noise for Opt.Filter with iterations")
            kwargs['NoiseScaleIndex'] = 2 # Noise for Optimal Filter with iterations
        else:
            msg.info("Adjusting TileInfo to return cell noise for Opt.Filter without iterations")
            kwargs['NoiseScaleIndex'] = 1 # Noise for Optimal Filter without iterations

        kwargs.setdefault('TileNoise', hasattr (flags, 'Digitization') and flags.Digitization.DoCaloNoise)
        if kwargs['TileNoise']:
            msg.info("Switching ON noise in Tile Digitization" )
        else:
            msg.info("Switching OFF noise in Tile Digitization" )


    TileInfoLoader=CompFactory.TileInfoLoader
    acc.addService(TileInfoLoader(**kwargs), primary = True, create=True)

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    
    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = ComponentAccumulator()
    acc.merge( TileInfoLoaderCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('TileInfoLoader'))
    acc.store( open('TileInfoLoader.pkl','wb') )

    print('All OK')
