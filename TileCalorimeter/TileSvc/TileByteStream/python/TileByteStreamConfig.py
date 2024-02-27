# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TileConfiguration.TileConfigFlags import TileRunType

def _createTileContByteStreamToolsConfig (name, TileContByteStreamTool, InitializeForWriting=False, stream=None, **kwargs):

    kwargs['name'] = name
    kwargs['InitializeForWriting'] = InitializeForWriting
    tool = TileContByteStreamTool(**kwargs)

    if InitializeForWriting:
        from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlg
        TileHid2RESrcIDCondAlg(ForHLT=True)

        if stream:
            stream.ExtraInputs |= {('TileHid2RESrcID', 'ConditionStore+TileHid2RESrcIDHLT')}

    return tool

def TileRawChannelContByteStreamToolConfig(name='TileRawChannelContByteStreamTool', InitializeForWriting=False, stream=None, **kwargs):
    from TileByteStream.TileByteStreamConf import TileRawChannelContByteStreamTool
    return _createTileContByteStreamToolsConfig(name, TileRawChannelContByteStreamTool, InitializeForWriting, stream, **kwargs)

def TileL2ContByteStreamToolConfig(name='TileL2ContByteStreamTool', InitializeForWriting=False, stream=None, **kwargs):
    from TileByteStream.TileByteStreamConf import TileL2ContByteStreamTool
    return _createTileContByteStreamToolsConfig(name, TileL2ContByteStreamTool, InitializeForWriting, stream, **kwargs)



# ComponentAccumulator version

def _createTileContByteStreamToolCfg (flags, name, InitializeForWriting=False, **kwargs):

    acc = ComponentAccumulator()
    TileContByteStreamTool = CompFactory.getComp(name)
    tool = TileContByteStreamTool(name, **kwargs)
    tool.InitializeForWriting = InitializeForWriting
    acc.addPublicTool(tool)

    extraOutputs = []
    if InitializeForWriting:
        from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
        acc.merge( TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True) )

        extraOutputs = [('TileHid2RESrcID', 'ConditionStore+TileHid2RESrcIDHLT')]

    return acc, extraOutputs


def TileDigitsContByteStreamToolCfg (flags,
                                     name="TileDigitsContByteStreamTool",
                                     InitializeForWriting=False,
                                     **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileRawChannelContByteStreamToolCfg (flags,
                                         name="TileRawChannelContByteStreamTool",
                                         InitializeForWriting=False,
                                         **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileMuRcvContByteStreamToolCfg (flags,
                                    name="TileMuRcvContByteStreamTool",
                                    InitializeForWriting=False,
                                    **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileL2ContByteStreamToolCfg (flags,
                                 name="TileL2ContByteStreamTool",
                                 InitializeForWriting=False,
                                 **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def TileLaserObjByteStreamToolCfg (flags,
                                   name="TileLaserObjByteStreamTool",
                                   InitializeForWriting=False,
                                   **kwargs):
    return _createTileContByteStreamToolCfg(flags, name, InitializeForWriting, **kwargs)

def addTileReadAlg(cfg, name, **kwargs):
    decoder = CompFactory.TileROD_Decoder(TileBadChanTool="", TileCondToolEmscale="")
    TileRawDataReadingAlg = CompFactory.TileRawDataReadingAlg
    cfg.addEventAlgo(TileRawDataReadingAlg(name, TileROD_Decoder=decoder, **kwargs))

def TileRawDataReadingCfg(flags, readDigits=True, readRawChannel=True,
                          readMuRcv=None, readMuRcvDigits=False, readMuRcvRawCh=False,
                          readBeamElem=None, readLaserObj=None, readDigitsFlx=False,
                          readL2=False, stateless=False, **kwargs):
    """
    Configure reading the Tile BS files

    Arguments:
      read[...] -- flag to read the corresponding Tile data from BS.
                   Possible values: None (default), True, False.
                   In the case of None it will be autoconfigured.
      stateless -- read online Tile data using emon BS service.
    """

    isPhysicsRun = flags.Tile.RunType is TileRunType.PHY
    isLaserRun = flags.Tile.RunType in [TileRunType.LAS, TileRunType.BILAS]
    isCalibRun = not isPhysicsRun

    # Set up default data
    readMuRcv = isPhysicsRun if readMuRcv is None else readMuRcv
    readBeamElem = isCalibRun if readBeamElem is None else readBeamElem
    readLaserObj = isLaserRun if readLaserObj is None else readLaserObj

    typeNames = kwargs.pop('type_names', [])

    prefix = flags.Overlay.BkgPrefix if flags.Overlay.DataOverlay else ''

    cfg = ComponentAccumulator()
    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    cfg.merge(TileCablingSvcCfg(flags))

    if stateless:
        from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
        cfg.merge( EmonByteStreamCfg(flags, type_names=typeNames) )
    else:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge( ByteStreamReadCfg(flags, type_names=typeNames) )

    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    cfg.merge( TileHid2RESrcIDCondAlgCfg(flags, ROD2ROBmap=['-1']) )

    if readDigits:
        addTileReadAlg(cfg, 'TileDigitsReadAlg', TileDigitsContainer=f'{prefix}TileDigitsCnt')
    if readRawChannel:
        addTileReadAlg(cfg, 'TileRawChannelReadAlg', TileRawChannelContainer=f'{prefix}TileRawChannelCnt')
    if readMuRcv:
        addTileReadAlg(cfg, 'TileMuRcvReadAlg', TileMuonReceiverContainer='TileMuRcvCnt')
    if readMuRcvDigits:
        addTileReadAlg(cfg, 'MuRcvDigitsReadAlg', MuRcvDigitsContainer=f'{prefix}MuRcvDigitsCnt')
    if readMuRcvRawCh:
        addTileReadAlg(cfg, 'TileMuRcvRawChReadAlg', MuRcvRawChannelContainer='MuRcvRawChCnt')
    if readLaserObj:
        addTileReadAlg(cfg, 'TileLaserObjReadAlg', TileLaserObject='TileLaserObj')
    if readBeamElem:
        addTileReadAlg(cfg, 'TileBeamElemReadAlg', TileBeamElemContainer='TileBeamElemCnt')
    if readDigitsFlx:
        addTileReadAlg(cfg, 'TileDigitsFlxReadAlg', TileDigitsFlxContainer='TileDigitsFlxCnt')
    if readL2:
        addTileReadAlg(cfg, 'TileL2ReadAlg', TileL2Container='TileL2Cnt')

    return cfg


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
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    cfg.merge( TileRawDataReadingCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    cfg.store( open('TileRawChannelReadAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
