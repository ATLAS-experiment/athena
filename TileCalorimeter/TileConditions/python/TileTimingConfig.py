# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile timing conditions tool and algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileTimingCondAlgCfg(flags, **kwargs):
    """Return component accumulator with configured Tile timing conditions algorithm

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        Source -- source of Tile timing conditions (COOL, FILE). Defaults to COOL.
        TileTiming -- name of Tile timing conditions object. Defaults to TileTiming.
        ForceOnline -- flag to use online timing in offline. Defaults to False.
    """

    acc = ComponentAccumulator()

    source = kwargs.get('Source', 'COOL')
    timing = kwargs.get('TileTiming', 'TileTiming')
    forceOnline = kwargs.get('ForceOnline', False)

    runType = flags.Tile.RunType
    timingType = runType.getTimingType().value

    name = timing + 'CondAlg'

    if source == 'COOL':
        # Connect COOL Tile conditions proxies to the algorithm (default)

        from TileConditions.TileFolders import TileFolders
        folders = TileFolders(isMC = flags.Input.isMC, isOnline = flags.Common.isOnline)

        if flags.IOVDb.DatabaseInstance == 'CONDBR2':
            onlineTimingFolder = '/TILE/ONL01/TIME/CHANNELOFFSET/' + timingType
        else:
            if timingType == 'GAP/LAS':
                raise(Exception("No Tile GAP/LAS timing in DB: %s" % flags.IOVDb.DatabaseInstance))

            onlineTimingFolder = '/TILE/OFL01/TIME/CHANNELOFFSET/' + timingType

        if forceOnline:
            timingFolder = folders.add(onlineTimingFolder, 'TILE')
        else:
            timingFolder = folders.addSplitOnline(onlineTimingFolder, '/TILE/OFL02/TIME/CHANNELOFFSET/' + timingType)

        TileCondProxyCoolFlt=CompFactory.getComp("TileCondProxyCool<TileCalibDrawerFlt>")
        timingProxy = TileCondProxyCoolFlt('TileCondProxyCool_AdcOffset', Source = timingFolder)

        from IOVDbSvc.IOVDbSvcConfig import addFolderList
        acc.merge( addFolderList(flags, folders.get()) )

    elif source == 'FILE':
        # Connect FILE Tile conditions proxies to the algorithm
        fileExtention = {'PHY' : 'tcphy', 'LAS' : 'tclas', 'CIS' : 'tccis'}

        TileCondProxyFileFlt=CompFactory.getComp("TileCondProxyFile<TileCalibDrawerFlt>")
        timingProxy = TileCondProxyFileFlt('TileCondProxyFile_AdcOffset',
                                          Source = 'TileDefault.' + fileExtention[timingType])
    else:
        raise(Exception("Invalid source: %s" % source))

    TileTimingCondAlg = CompFactory.getComp("TileCondAlg<TileTiming,TileCalibDrawerFlt>")
    timingCondAlg = TileTimingCondAlg( name = name,
                                       ConditionsProxy = timingProxy,
                                       TileCondData = timing)

    acc.addCondAlgo(timingCondAlg)

    return acc



def TileCondToolTimingCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile timing conditions tool

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        Source -- source of Tile timing conditions (COOL, FILE). Defaults to COOL.
        TileTiming -- name of Tile timing conditions object. Defaults to TileTiming (TileOnlineTiming if ForceOnline).
        TimingType -- type of Tile timing. Defaults to run type (PED->PHY, BILAS->LAS, MONOCIS->CIS).
                      Possible Tile timing types: PHY, LAS, GAP/LAS, CIS.
        ForceOnline -- flag to use online timing in offline. Defaults to False.
    """

    acc = ComponentAccumulator()

    kwargs.setdefault('Source', 'COOL')
    kwargs.setdefault('ForceOnline', False)

    forceOnline = kwargs['ForceOnline']
    kwargs.setdefault('TileTiming', 'TileOnlineTiming' if forceOnline else 'TileTiming')

    timing = kwargs['TileTiming']

    name = 'TileCondToolTiming'

    acc.merge( TileTimingCondAlgCfg(flags, **kwargs) )

    TileCondToolTiming=CompFactory.TileCondToolTiming
    acc.setPrivateTools( TileCondToolTiming(name, TileTiming = timing) )

    return acc



def TileCondToolOnlineTimingCfg(flags, **kwargs):
    return TileCondToolTimingCfg(flags, ForceOnline = True)

if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()

    acc = ComponentAccumulator()

    timingTool = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )
    print(timingTool)

    onlineTimingTool = acc.popToolsAndMerge( TileCondToolOnlineTimingCfg(flags) )
    print(onlineTimingTool)

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('IOVDbSvc'))
    acc.store( open('TileTiming.pkl','wb') )

    print('All OK')

