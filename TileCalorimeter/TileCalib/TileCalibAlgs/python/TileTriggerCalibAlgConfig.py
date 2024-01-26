#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTriggerCalibAlgConfig.py
@brief Python configuration of TileTriggerDefaultCalibTool tool for the Run III
'''

from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TileConfiguration.TileConfigFlags import TileRunType

def TileTriggerDefaulCalibToolCfg(flags, **kwargs):

    ''' Function to configure TileTriggerDefaultCalibTool tool'''

    kwargs.setdefault('TileRawChannelContainer', 'TileRawChannelFit')
    kwargs.setdefault('TriggerTowerContainer', 'xAODTriggerTowers')

    acc = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg( flags ))

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    # For running on bytestream data
    if flags.Input.Format is Format.BS:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags, type_names=['xAOD::TriggerTowerContainer/xAODTriggerTowers',
                                                       'xAOD::TriggerTowerAuxContainer/xAODTriggerTowersAux.']))

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileTriggerDefalutCalibTool = CompFactory.TileTriggerDefaultCalibTool

    acc.setPrivateTools(TileTriggerDefalutCalibTool(**kwargs))

    return acc


'''
@brief Python configuration of TileTriggerCalibAlg algorithm for the Run III
'''
def TileTriggerCalibAlgCfg(flags, **kwargs):

    ''' Function to configure TileTriggerCalibAlg algorithm'''

    kwargs.setdefault('name', 'TileTriggerCalibAlg')
    kwargs.setdefault('RunType', TileRunType.CIS.getIntValue())
    kwargs.setdefault('RunNumber', flags.Input.RunNumbers[0])

    acc = ComponentAccumulator()

    if 'TileCalibTools' not in kwargs:
        triggerCalibTool = acc.popToolsAndMerge( TileTriggerDefaulCalibToolCfg(flags) )
        kwargs['TileCalibTools'] = [triggerCalibTool]

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

    cfg.merge( TileTriggerCalibAlgCfg(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileTriggerCalibAlg.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
