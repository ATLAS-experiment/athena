# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured TileHid2ReSrcIDCondAlg conditions algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders

def TileHid2RESrcIDCondAlg(ForHLT=False, **kwargs):
    """Configure TileHid2ReSrcIDCondAlg conditions algorithm"""

    hid2RESrcID = 'TileHid2RESrcIDHLT' if ForHLT else 'TileHid2RESrcID'
    hid2RESrcIDCondAlg = f'{hid2RESrcID}CondAlg'
    from AthenaCommon.AlgSequence import AthSequencer
    condSequence = AthSequencer("AthCondSeq")
    if hasattr(condSequence, hid2RESrcIDCondAlg):
        return #Already there....

    from TileByteStream.TileByteStreamConf import TileHid2RESrcIDCondAlg
    condSequence += TileHid2RESrcIDCondAlg(name=hid2RESrcIDCondAlg, ForHLT=ForHLT, TileHid2RESrcID=hid2RESrcID, **kwargs)
    return


def TileHid2RESrcIDCondAlgCfg(flags, source='COOL', **kwargs):
    """Return component accumulator with configured TileHid2ReSrcIDCondAlg conditions algorithm"""

    forHLT = kwargs.get('ForHLT', False)
    hid2RESrcID = 'TileHid2RESrcIDHLT' if forHLT else 'TileHid2RESrcID'
    kwargs.setdefault('TileHid2RESrcID', hid2RESrcID)
    kwargs.setdefault('name', f'{hid2RESrcID}CondAlg')
    
    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg(flags) )

    if source == 'COOL':
        rodFolder = None
        # Connect COOL Tile conditions proxies to the tool
        if flags.IOVDb.DatabaseInstance == 'COMP200':
            # In the COMP200 DB there is no conditions with ROD status
            kwargs['FullTileMode'] = 1  # Configure predifined mapping for Run1
        elif flags.Input.isMC:
            rodFolder = '/TILE/OFL02/STATUS/ROD'
            # Temporary tag is hardcoded until it is connected to the global tags
            acc.merge(addFolders(flags, rodFolder, 'TILE_OFL', tag='TileOfl02StatusRod-RUN3-00', className='CondAttrListCollection'))
        else:
            rodFolder = '/TILE/ONL01/STATUS/ROD'
            acc.merge(addFolders(flags, rodFolder, 'TILE', className='CondAttrListCollection'))

        if rodFolder:
            TileCondProxyCoolInt = CompFactory.getComp("TileCondProxyCool<TileCalibDrawerInt>")
            rodStatusProxy = TileCondProxyCoolInt('TileCondProxyCool_ROD', Source=rodFolder)
            kwargs['RODStatusProxy'] = rodStatusProxy

    elif source == 'FILE':
        TileCondProxyFileInt = CompFactory.getComp("TileCondProxyFile<TileCalibDrawerInt>")
        rodStatusProxy = TileCondProxyFileInt('TileCondProxyFile_ROD', Source='TileDefault.fullrod')
        kwargs['RODStatusProxy'] = rodStatusProxy

    TileHid2ReSrcIDCondAlg = CompFactory.TileHid2RESrcIDCondAlg
    acc.addCondAlgo( TileHid2ReSrcIDCondAlg(**kwargs) )

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    
    # Test setup
    log.setLevel(INFO)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge( ByteStreamReadCfg(flags) )

    cfg.merge( TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    cfg.store( open('TileHid2ReSrcIDCondAlg.pkl','wb') )

    sc = cfg.run(3)

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
