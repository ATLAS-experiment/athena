# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile TMDB conditions tool and algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileTMDBCondAlgCfg(flags, **kwargs):
    """Return component accumulator with configured Tile TMDB conditions algorithms

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        Source -- source of Tile TMDB conditions (COOL, FILE). Defaults to COOL.
        TileTMDBThreshold -- name of Tile TMDB threshold conditions object. Defaults to TileTMDBThreshold.
        TileTMDBDelay -- name of Tile TMDB dealy conditions object. Defaults to TileTMDBDelay.
        TileTMDBTMF -- name of Tile TMDB TMF conditions object. Defaults to TileTMDBTMF.
        TileTMDBCalib -- name of Tile TMDB calibration conditions object. Defaults to TileTMDBCalib.
    """

    acc = ComponentAccumulator()

    source = kwargs.get('Source', 'COOL')
    threshod = kwargs.get('TileTMDBThreshold', 'TileTMDBThreshold')
    delay = kwargs.get('TileTMDBDelay', 'TileTMDBDelay')
    tmf = kwargs.get('TileTMDBTMF', 'TileTMDBTMF')
    calib = kwargs.get('TileTMDBCalib', 'TileTMDBCalib')

    TileCalibFltCondAlg=CompFactory.getComp("TileCalibCondAlg<TileCalibDrawerFlt>")
    def __addCondAlg(calibData, proxy):
        condAlg = TileCalibFltCondAlg( name = calibData + 'CondAlg',
                                       ConditionsProxy = proxy,
                                       TileCalibData = calibData)
        acc.addCondAlgo(condAlg)

    if source == 'COOL':
        # Connect COOL Tile conditions proxies to the algorithm (default)

        from TileConditions.TileFolders import TileFolders
        folders = TileFolders(isMC = flags.Input.isMC, isOnline = flags.Common.isOnline)

        runType = flags.Tile.RunType
        run = runType.getCommonType().value

        if flags.IOVDb.DatabaseInstance  == 'CONDBR2':
            thrFolder = folders.addSplitMC('/TILE/ONL01/TMDB/THRESHOLD/' + run, '/TILE/ONL01/TMDB/THRESHOLD/' + run)
            delayFolder = folders.addSplitMC('/TILE/ONL01/TMDB/DELAY/' + run, '/TILE/ONL01/TMDB/DELAY/' + run)
            tmfFolder = folders.addSplitMC('/TILE/ONL01/TMDB/TMF/' + run, '/TILE/ONL01/TMDB/TMF/' + run)
            calibFolder = folders.addSplitMC('/TILE/ONL01/TMDB/CALIB/' + run, '/TILE/ONL01/TMDB/CALIB/' + run)
        else:
            raise(Exception("No Tile TMDB folders in %s" % flags.IOVDb.DatabaseInstancea))

        TileCondProxyCoolFlt=CompFactory.getComp("TileCondProxyCool<TileCalibDrawerFlt>")
        thrProxy = TileCondProxyCoolFlt('TileCondProxyCool_TMDBThreshold', Source = thrFolder)
        delayProxy = TileCondProxyCoolFlt('TileCondProxyCool_TMDBDelay', Source = delayFolder)
        tmfProxy = TileCondProxyCoolFlt('TileCondProxyCool_TMDBTmf', Source = tmfFolder)
        calibProxy = TileCondProxyCoolFlt('TileCondProxyCool_TMDBCalib', Source = calibFolder)
        
        from IOVDbSvc.IOVDbSvcConfig import addFolderList
        acc.merge( addFolderList(flags, folders.get()) )

    else:
        # Connect FILE Tile conditions proxies to the algorithm
        raise(Exception("Not implemented source %s" % source))


    __addCondAlg(threshod, thrProxy)
    __addCondAlg(delay, delayProxy)
    __addCondAlg(tmf, tmfProxy)
    __addCondAlg(calib, calibProxy)

    return acc


def TileCondToolTMDBCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile TMDB conditions tool

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        Source -- source of Tile TMDB conditions (COOL, FILE). Defaults to COOL.
        TileTMDBThreshold -- name of Tile TMDB threshold conditions object. Defaults to TileTMDBThreshold.
        TileTMDBDelay -- name of Tile TMDB dealy conditions object. Defaults to TileTMDBDelay.
        TileTMDBTMF -- name of Tile TMDB TMF conditions object. Defaults to TileTMDBTMF.
        TileTMDBCalib -- name of Tile TMDB calibration conditions object. Defaults to TileTMDBCalib.
    """

    acc = ComponentAccumulator()

    kwargs.setdefault('Source', 'COOL')
    kwargs.setdefault('TileTMDBThreshold', 'TileTMDBThreshold')
    kwargs.setdefault('TileTMDBDelay', 'TileTMDBDelay')
    kwargs.setdefault('TileTMDBTMF', 'TileTMDBTMF')
    kwargs.setdefault('TileTMDBCalib', 'TileTMDBCalib')

    threshod = kwargs['TileTMDBThreshold']
    delay = kwargs['TileTMDBDelay']
    tmf = kwargs['TileTMDBTMF']
    calib = kwargs['TileTMDBCalib']

    acc.merge( TileTMDBCondAlgCfg(flags, **kwargs) )

    TileCondToolTMDB=CompFactory.TileCondToolTMDB
    acc.setPrivateTools( TileCondToolTMDB(TileTMDBThreshold = threshod,
                                          TileTMDBDelay = delay,
                                          TileTMDBTMF = tmf,
                                          TileTMDBCalib = calib) )

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
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()

    acc = ComponentAccumulator()

    tmdbTool = acc.popToolsAndMerge( TileCondToolTMDBCfg(flags) )
    print(tmdbTool)

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('IOVDbSvc'))
    acc.store( open('TileTMDB.pkl','wb') )

    print('All OK')

