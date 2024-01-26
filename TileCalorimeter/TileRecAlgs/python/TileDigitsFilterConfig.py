# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile digits filter algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileDigitsFilterCfg(flags, **kwargs):
    """Return component accumulator with configured Tile digits filter algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    acc.merge(DetDescrCnvSvcCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    TileDigitsFilter=CompFactory.TileDigitsFilter
    acc.addEventAlgo(TileDigitsFilter(**kwargs), primary = True)

    return acc



def TileDigitsFilterOutputCfg(flags, streamName = 'ESD', **kwargs):

    acc = TileDigitsFilterCfg(flags)
    tileDigitsFilter = acc.getPrimary()

    outputItemList = []

    if 'OutputDigitsContainer' in tileDigitsFilter._properties:
        digitsContainer = str(tileDigitsFilter._properties['OutputDigitsContainer'])
    else:
        digitsContainer = str(tileDigitsFilter._descriptors['OutputDigitsContainer'].default)

    if digitsContainer != '':
        digitsContainer = digitsContainer.replace('StoreGateSvc+', '')
        outputItemList += ['TileDigitsContainer#' + digitsContainer]

    if 'OutputRawChannelContainer' in tileDigitsFilter._properties:
        rawChannelContainer = str(tileDigitsFilter._properties['OutputRawChannelContainer'])
    else:
        rawChannelContainer = str(tileDigitsFilter._descriptors['OutputRawChannelContainer'].default)

    if rawChannelContainer != '':
        rawChannelContainer = rawChannelContainer.replace('StoreGateSvc+', '')
        outputItemList += ['TileRawChannelContainer#' + rawChannelContainer]

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(  OutputStreamCfg(flags, streamName, ItemList = outputItemList) )

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    acc.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )

    acc.merge( TileDigitsFilterOutputCfg(flags) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileDigitsFilter.pkl','wb') )

    sc = acc.run(maxEvents = 3)

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

