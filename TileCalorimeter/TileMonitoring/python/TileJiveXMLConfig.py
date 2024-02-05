#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
import os

'''
@file TileAtlantisConfig.py
@brief Python configuration of Atlantis algorithm for Tile for the Run III
'''


def getTileDigitsContainer(flags):
    ''' Function to get Tile digits container to be used in Atlantis.'''

    tileDigitsContainer = "TileDigitsCnt"

    if flags.Input.Format is Format.POOL:
        if "TileDigitsCnt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsCnt"
        elif "TileDigitsFlt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsFlt"

    return tileDigitsContainer


def getTileRawChannelContainer(flags):
    ''' Function to get Tile raw channel container to be used in Atlantis.'''

    tileRawChannelContainer = ""

    if flags.Input.Format is Format.BS and flags.Tile.readDigits:
        if flags.Tile.doOpt2:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif flags.Tile.doOptATLAS:
            tileRawChannelContainer = 'TileRawChannelFixed'
        elif flags.Tile.doFitCOOL:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif flags.Tile.doFit:
            tileRawChannelContainer = 'TileRawChannelFit'
    else:
        if "TileRawChannelOpt2" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif "TileRawChannelFitCool" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif "TileRawChannelFit" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFit'
        elif "TileRawChannelCnt" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelCnt'

    return tileRawChannelContainer


def TileAlgoJiveXMLCfg(flags, TileDigitsContainer=None, TileRawChannelContainer=None,
                       CaloClusterContainer='TileTopoCluster', **kwargs):
    """
    Function to configure AlgoJiveXML algorithm for Tile.

    Arguments:
      TileDigitsContainer     -- name of Tile digits container (default: None).
                                 In the case of None it will be autoconfigured.
                                 In the case of empty string it will not be used.
      TileRawChannelContainer -- name of Tile raw channel container (default: None).
                                 In the case of None it will be autoconfigured.
                                 In the case of empty string it will not be used.
      CaloClusterContainer    -- name of Calo cluster container (default: TileTopoCluster).
                                 In the case of empty string it will not be used.
    """

    data_types = []  # These data types need to match the tools added later

    if TileDigitsContainer is None:
        digitsContainer = getTileDigitsContainer(flags)
    else:
        digitsContainer = TileDigitsContainer

    if TileRawChannelContainer is None:
        rawChannelContainer = getTileRawChannelContainer(flags)
    else:
        rawChannelContainer = TileRawChannelContainer

    useDigits = digitsContainer != ""
    useRawChannels = rawChannelContainer != ""
    useClusters = CaloClusterContainer != ""

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('TileAlgoJiveXMLCfg')

    if useDigits:
        msg.info(f'Use TileDigitsContainer [{digitsContainer}]')
    else:
        msg.info('TileDigitsContainer is not configured => no use of digits')

    if useRawChannels:
        msg.info(f'Use TileRawChannelContainer [{rawChannelContainer}] for detailed information')
    else:
        msg.info('TileRawChannelContainer is not configured => no detailed cell information')

    if useClusters:
        msg.info(f'Use CaloClusterContainer [{CaloClusterContainer}]')
    else:
        msg.info('CaloClusterContainer is not configured => no clusters information')

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    from TileConditions.TileTimingConfig import TileTimingCondAlgCfg
    acc.merge( TileTimingCondAlgCfg(flags) )

    if useClusters:
        from TileMonitoring.TileTopoClusterConfig import TileTopoClusterCfg
        acc.merge( TileTopoClusterCfg(flags) )

    data_types += ["JiveXML::CaloTileRetriever/CaloTileRetriever"]
    acc.addPublicTool(
        CompFactory.JiveXML.CaloTileRetriever(
            name="CaloTileRetriever",
            TileDigitsContainer=digitsContainer,
            TileRawChannelContainer=rawChannelContainer,
            DoTileCellDetails=useRawChannels,
            DoTileDigit=useDigits,
            DoBadTile=False,
            CellThreshold=50.0
        )
    )

    data_types += ["JiveXML::CaloMBTSRetriever/CaloMBTSRetriever"]
    acc.addPublicTool(
        CompFactory.JiveXML.CaloMBTSRetriever(
            name="CaloMBTSRetriever",
            TileDigitsContainer=digitsContainer,
            TileRawChannelContainer=rawChannelContainer,
            DoMBTSDigits=useDigits,
            DoMBTSCellDetails=useRawChannels,
            MBTSThreshold=0.05
        )
    )

    if useClusters:
        data_types += ["JiveXML::CaloClusterRetriever/CaloClusterRetriever"]
        acc.addPublicTool(
            CompFactory.JiveXML.CaloClusterRetriever(
                name="CaloClusterRetriever",
                FavouriteClusterCollection="TileTopoCluster",
                OtherClusterCollections=["TileTopoCluster"]
            )
        )

    kwargs.setdefault("AtlasRelease", os.environ.get("AtlasVersion", "Unknown"))
    kwargs.setdefault("WriteToFile", True)
    kwargs.setdefault("OnlineMode", False)
    ### Enable this to recreate the geometry XML files for Atlantis
    kwargs.setdefault("WriteGeometry", False)
    kwargs.setdefault("DataTypes", data_types)

    AlgoJiveXML = CompFactory.JiveXML.AlgoJiveXML
    acc.addEventAlgo(AlgoJiveXML(name="AlgoJiveXML", **kwargs), primary=True)

    return acc


if __name__ == '__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge( TileAlgoJiveXMLCfg(flags) )

    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
